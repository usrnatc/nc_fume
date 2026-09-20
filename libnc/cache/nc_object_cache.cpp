OCShared* OC_SHARED = NULL;

void 
OCInit(void)
{
    Arena* MemPool = ArenaAlloc();

    OC_SHARED = ArenaPushArrayZero(MemPool, OCShared, 1);
    OC_SHARED->MemPool = MemPool;
    OC_SHARED->CacheSlotsCount = 256;
    OC_SHARED->CacheSlots = ArenaPushArrayZero(
        MemPool, 
        OCCache*, 
        OC_SHARED->CacheSlotsCount
    );
    OC_SHARED->CacheStripes = StripeArrayAlloc(MemPool);

    for (
        u32 Index = 0; 
        Index < ARRAY_COUNT(OC_SHARED->RequestBatches); 
        ++Index
    ) {
        OC_SHARED->RequestBatches[Index].Mtx = MutexAlloc();
        OC_SHARED->RequestBatches[Index].MemPool = ArenaAlloc();
    }

    OC_SHARED->CancelThread = ThreadLaunch(OCCancelThreadEntryPoint, NULL);
    OC_SHARED->CancelThreadSemaphore = SemaphoreAlloc(0, 1, {});
}

OCObject 
__OCObjectFromKey(Access* Acc, Str8 Key, OCObjectParams* Params, u64 EndTimeUSecs)
{
    OCRequestBatch* RequestBatch = &OC_SHARED->RequestBatches[Params->Flags & OC_HIGH_PRIORITY ? 0 : 1];
    OCCache* Cache = NULL;
    u64 CacheHash = Hash(Str8Struct(&Params->CreateFunc));
    u64 CacheSlotIndex = CacheHash % OC_SHARED->CacheSlotsCount;
    Stripe* CacheStripe = StripeFromSlotIndex(&OC_SHARED->CacheStripes, CacheSlotIndex);

    LOCK_SCOPE_R(CacheStripe->RWMtx) {
        for (OCCache* C = OC_SHARED->CacheSlots[CacheSlotIndex]; C; C = C->Next) {
            if (C->CreateFunc == Params->CreateFunc) {
                Cache = C;
                break;
            }
        }
    }

    if (!Cache) {
        LOCK_SCOPE_W(CacheStripe->RWMtx) {
            for (OCCache* C = OC_SHARED->CacheSlots[CacheSlotIndex]; C; C = C->Next) {
                if (C->CreateFunc == Params->CreateFunc) {
                    Cache = C;
                    break;
                }
            }

            if (!Cache) {
                Cache = ArenaPushArrayZero(CacheStripe->MemPool, OCCache, 1);
                SLL_STACK_PUSH(OC_SHARED->CacheSlots[CacheSlotIndex], Cache);
                Cache->CreateFunc = Params->CreateFunc;
                Cache->DestroyFunc = Params->DestroyFunc;
                Cache->SlotsCount = MAX(256, Params->SlotsCount);
                Cache->Slots = ArenaPushArrayZero(
                    CacheStripe->MemPool, 
                    OCSlot, 
                    Cache->SlotsCount
                );
                Cache->Stripes = StripeArrayAlloc(CacheStripe->MemPool);
            }
        }
    }

    u64 KeyHash = Hash(Key);
    u64 SlotIndex = KeyHash % Cache->SlotsCount;
    OCSlot* Slot = &Cache->Slots[SlotIndex];
    Stripe* S = StripeFromSlotIndex(&Cache->Stripes, SlotIndex);
    b32 ObjectIsStale = TRUE;
    b32 HaveObject = FALSE;
    b32 NeedRequest = FALSE;
    OCObject Object = {};

    LOCK_SCOPE_R(S->RWMtx) {
        for (OCNode* Node = Slot->Head; Node; Node = Node->Next) {
            if (StrMatch(Node->Key, Key, 0)) {
                AtomicExchangeU64(
                    &Node->LastRequestedGeneration, 
                    Params->Generation, 
                    MEM_ORDER_SEQ_CST
                );

                b32 IsStale = (Node->LastCompletedGeneration != Params->Generation);

                if (
                    AtomicLoadU64(&Node->CompletionCount, MEM_ORDER_SEQ_CST) && 
                    (
                        !IsStale || 
                        !(Params->Flags & OC_WAIT_FOR_FRESH)
                    )
                ) {
                    HaveObject = TRUE;
                    ObjectIsStale = IsStale;
                    Object = Node->V;
                    AccessTouch(Acc, &Node->Point, S->CondVar);
                }

                if (IsStale) {
                    b32 GotTask = !AtomicCASU64(
                        &Node->WorkingCount, 
                        1, 
                        0, 
                        MEM_ORDER_SEQ_CST, 
                        MEM_ORDER_SEQ_CST
                    );

                    NeedRequest = GotTask;
                }

                break;
            }
        }
    }

    if (!HaveObject || NeedRequest) {
        LOCK_SCOPE_W(S->RWMtx) {
            for (;;) {
                b32 OutOfTime = (TimeNow() >= EndTimeUSecs);
                OCNode* Node = NULL;

                for (OCNode* N = Slot->Head; N; N = N->Next) {
                    if (StrMatch(N->Key, Key, 0)) {
                        Node = N;
                        break;
                    }
                }

                if (!Node) {
                    NeedRequest = TRUE;
                    Node = (OCNode*) S->Free;

                    if (Node)
                        S->Free = Node->Next;
                    else
                        Node = ArenaPushArray(S->MemPool, OCNode, 1);

                    MemSet(Node, 0, sizeof(*Node));
                    DLL_PUSH_BACK(Slot->Head, Slot->Tail, Node);
                    Node->Key = ArenaPushStrCpy(S->MemPool, Key);
                    Node->WorkingCount = 1;
                    Node->EvictionThresholdUSecs = Params->EvictionThresholdUSecs;
                }

                Node->Point.LastTimeTouchedUSecs = TimeNow();
                Node->Point.LastUpdateIndexTouched = UpdateTickIndex();

                if (NeedRequest) {
                    NeedRequest = FALSE;

                    LOCK_SCOPE(RequestBatch->Mtx) {
                        OCRequestNode* N = ArenaPushArrayZero(RequestBatch->MemPool, OCRequestNode, 1);

                        if (Params->Flags & OC_WIDE) {
                            SLL_QUEUE_PUSH(RequestBatch->HeadWide, RequestBatch->TailWide, N);
                            ++RequestBatch->WideCount;
                        } else {
                            SLL_QUEUE_PUSH(RequestBatch->HeadThin, RequestBatch->TailThin, N);
                            ++RequestBatch->ThinCount;
                        }

                        N->V.Key = ArenaPushStrCpy(RequestBatch->MemPool, Key);
                        N->V.Generation = Params->Generation;
                        N->V.CancelSignal = &Node->Cancelled;
                        N->V.CreateFunc = Params->CreateFunc;
                    }

                    CondVarBroadcast(ASYNC_TICK_BEGIN_COND_VAR);
                    AtomicExchangeU32(&ASYNC_LOOP_REPEAT, TRUE, MEM_ORDER_SEQ_CST);

                    if (Params->Flags & OC_HIGH_PRIORITY)
                        AtomicExchangeU32(&ASYNC_LOOP_REPEAT_HIGH_PRIORITY, TRUE, MEM_ORDER_SEQ_CST);
                }

                if (
                    !HaveObject && 
                    AtomicLoadU64(&Node->CompletionCount, MEM_ORDER_SEQ_CST) && 
                    (
                        (Node->LastCompletedGeneration == Params->Generation) || 
                        !(Params->Flags & OC_WAIT_FOR_FRESH) || 
                        OutOfTime
                    )
                ) {
                    HaveObject = TRUE;
                    ObjectIsStale = (Node->LastCompletedGeneration != Params->Generation);
                    Object = Node->V;
                    AccessTouch(Acc, &Node->Point, S->CondVar);
                }

                if (OutOfTime || HaveObject || GetTLS()->IS_ASYNC_THREAD)
                    break;

                CondVarWaitW(S->CondVar, S->RWMtx, EndTimeUSecs);
            }
        }
    }

    if (Params->Stale)
        *Params->Stale = ObjectIsStale;

    return Object;
}

void 
OCAsyncTick(void)
{
    TempArena Scratch = GetScratch(NULL, 0);

    if (!LaneIndex())
        SemaphoreUnlock(OC_SHARED->CancelThreadSemaphore);

    for (
        u64 CacheSlotIndex = 0; 
        CacheSlotIndex < OC_SHARED->CacheSlotsCount; 
        ++CacheSlotIndex
    ) {
        Stripe* CacheStripe = StripeFromSlotIndex(&OC_SHARED->CacheStripes, CacheSlotIndex);

        LOCK_SCOPE_R(CacheStripe->RWMtx) {
            for (
                OCCache* Cache = OC_SHARED->CacheSlots[CacheSlotIndex]; 
                Cache; 
                Cache = Cache->Next
            ) {
                r1u64 SlotRange = LaneRange(Cache->SlotsCount);

                for (
                    u64 SlotIndex = SlotRange.Min; 
                    SlotIndex < SlotRange.Max; 
                    ++SlotIndex
                ) {
                    OCSlot* Slot = &Cache->Slots[SlotIndex];
                    Stripe* S = StripeFromSlotIndex(&Cache->Stripes, SlotIndex);

                    for (b32 WriteMode = FALSE; WriteMode <= TRUE; ++WriteMode) {
                        b32 SlotHasWork = FALSE;

                        if (!WriteMode) {
                            LOCK_SCOPE_R(S->RWMtx) {
                                for (OCNode* N = Slot->Head; N; N = N->Next) {
                                    if (
                                        AccessPointIsExpired(&N->Point, __Params.Time = N->EvictionThresholdUSecs) &&
                                        !AtomicLoadU64(&N->WorkingCount, MEM_ORDER_SEQ_CST)
                                    ) {
                                        SlotHasWork = TRUE;
                                        break;
                                    }
                                }
                            }
                        } else {
                            LOCK_SCOPE_W(S->RWMtx) {
                                for (
                                    OCNode* N = Slot->Head, *Next = NULL; 
                                    N; 
                                    N = Next
                                ) {
                                    Next = N->Next;

                                    if (
                                        AccessPointIsExpired(&N->Point, __Params.Time = N->EvictionThresholdUSecs) &&
                                        !AtomicLoadU64(&N->WorkingCount, MEM_ORDER_SEQ_CST)
                                    ) {
                                        SlotHasWork = TRUE;
                                        DLL_REMOVE(Slot->Head, Slot->Tail, N);
                                        N->Next = (OCNode*) S->Free;
                                        S->Free = N;

                                        if (Cache->DestroyFunc)
                                            Cache->DestroyFunc(N->V);
                                    }
                                }
                            }
                        }

                        if (!SlotHasWork)
                            break;
                    }
                }
            }
        }
    }

    struct RequestBatchTask {
        OCRequest* Wide;
        u64        WideCount;
        OCRequest* Thin;
        u64        ThinCount;
    };

    RequestBatchTask* Tasks = NULL;
    u64 TasksCount = 0;

    if (!LaneIndex()) {
        TasksCount = ARRAY_COUNT(OC_SHARED->RequestBatches);
        Tasks = ArenaPushArrayZero(Scratch.MemPool, RequestBatchTask, TasksCount);

        for (u64 TaskIndex = 0; TaskIndex < TasksCount; ++TaskIndex) {
            OCRequestBatch* Batch = &OC_SHARED->RequestBatches[TaskIndex];

            LOCK_SCOPE(Batch->Mtx) {
                Tasks[TaskIndex].WideCount = Batch->WideCount;
                Tasks[TaskIndex].ThinCount = Batch->ThinCount;
                Tasks[TaskIndex].Wide = ArenaPushArray(
                    Scratch.MemPool, 
                    OCRequest, 
                    Tasks[TaskIndex].WideCount
                );
                Tasks[TaskIndex].Thin = ArenaPushArray(
                    Scratch.MemPool, 
                    OCRequest, 
                    Tasks[TaskIndex].ThinCount
                );

                u64 Index = 0;

                for (OCRequestNode* N = Batch->HeadWide; N; N = N->Next) {
                    MemCpy(&Tasks[TaskIndex].Wide[Index], &N->V, sizeof(N->V));
                    Tasks[TaskIndex].Wide[Index].Key = ArenaPushStrCpy(
                        Scratch.MemPool, 
                        Tasks[TaskIndex].Wide[Index].Key
                    );
                    ++Index;
                }

                Index = 0;

                for (OCRequestNode* N = Batch->HeadThin; N; N = N->Next) {
                    MemCpy(&Tasks[TaskIndex].Thin[Index], &N->V, sizeof(N->V));
                    Tasks[TaskIndex].Thin[Index].Key = ArenaPushStrCpy(
                        Scratch.MemPool, 
                        Tasks[TaskIndex].Thin[Index].Key
                    );
                    ++Index;
                }

                ArenaClear(Batch->MemPool);
                Batch->HeadWide = NULL;
                Batch->TailWide = NULL;
                Batch->HeadThin = NULL;
                Batch->TailThin = NULL;
                Batch->WideCount = 0;
                Batch->ThinCount = 0;
            }
        }
    }

    LaneSyncU64(&Tasks, 0);
    LaneSyncU64(&TasksCount, 0);
    LaneSync();

    for (u64 TaskIndex = 0; TaskIndex < TasksCount; ++TaskIndex) {
        LaneSync();

        RequestBatchTask* Task = &Tasks[TaskIndex];
        u64 Cancelled = 0;
        u64* CancelledPtr = NULL;

        if (!LaneIndex())
            CancelledPtr = &Cancelled;

        LaneSyncU64(&CancelledPtr, 0);

        u64 DoneWideCount = 0;

        for (u64 Index = 0; Index < Task->WideCount; ++Index) {
            LaneSync();

            OCRequest* R = &Task->Wide[Index];

            if (LaneIndex() == 0) {
                if (
                    TaskIndex == 1 && 
                    Index && 
                    AtomicLoadU32(&ASYNC_LOOP_REPEAT_HIGH_PRIORITY, MEM_ORDER_SEQ_CST)
                ) {
                    AtomicExchangeU64(CancelledPtr, 1, MEM_ORDER_SEQ_CST);
                }
            }

            LaneSync();

            if (AtomicLoadU64(CancelledPtr, MEM_ORDER_SEQ_CST))
                break;

            b32 Retry = FALSE;
            u64 Gen = R->Generation;
            OCObject Val = R->CreateFunc(R->Key, R->CancelSignal, &Retry, &Gen);

            if (
                Retry && 
                !LaneIndex() &&
                !AtomicLoadU32(R->CancelSignal, MEM_ORDER_SEQ_CST)
            ) {
                OCRequestBatch* Batch = &OC_SHARED->RequestBatches[TaskIndex];

                LOCK_SCOPE(Batch->Mtx) {
                    OCRequestNode* N = ArenaPushArray(Batch->MemPool, OCRequestNode, 1);

                    SLL_QUEUE_PUSH(Batch->HeadWide, Batch->TailWide, N);
                    ++Batch->WideCount;
                    MemCpy(&N->V, R, sizeof(*R));
                    N->V.Key = ArenaPushStrCpy(Batch->MemPool, N->V.Key);
                }

                AtomicExchangeU32(&ASYNC_LOOP_REPEAT, TRUE, MEM_ORDER_SEQ_CST);
            }

            OCCache* Cache = NULL;

            if (!Retry && !LaneIndex()) {
                u64 CacheHash = Hash(Str8Struct(&R->CreateFunc));
                u64 CacheSlotIndex = CacheHash % OC_SHARED->CacheSlotsCount;
                Stripe* CacheStripe = StripeFromSlotIndex(
                    &OC_SHARED->CacheStripes, 
                    CacheSlotIndex
                );

                LOCK_SCOPE_R(CacheStripe->RWMtx) {
                    for (
                        OCCache* C = OC_SHARED->CacheSlots[CacheSlotIndex]; 
                        C; 
                        C = C->Next
                    ) {
                        if (C->CreateFunc == R->CreateFunc) {
                            Cache = C;
                            break;
                        }
                    }
                }
            }

            if (!Retry && !LaneIndex()) {
                u64 KeyHash = Hash(R->Key);
                u64 SlotIndex = KeyHash % Cache->SlotsCount;
                OCSlot* Slot = &Cache->Slots[SlotIndex];
                Stripe* S = StripeFromSlotIndex(&Cache->Stripes, SlotIndex);

                LOCK_SCOPE_W(S->RWMtx) {
                    for (OCNode* N = Slot->Head; N; N = N->Next) {
                        if (StrMatch(N->Key, R->Key, 0)) {
                            N->LastCompletedGeneration = Gen;
                            N->V = Val;
                            AtomicSubFetchU64(&N->WorkingCount, 1, MEM_ORDER_SEQ_CST);
                            AtomicAddFetchU64(&N->CompletionCount, 1, MEM_ORDER_SEQ_CST);
                            break;
                        }
                    }
                }

                CondVarBroadcast(S->CondVar);
            }

            LaneSync();
            ++DoneWideCount;
        }

        LaneSync();

        u64 DoneThinCount = 0;
        u64 ReqTakeCounter = 0;
        u64* ReqTakeCounterPtr = NULL;

        if (!LaneIndex())
            ReqTakeCounterPtr = &ReqTakeCounter;

        LaneSyncU64(&ReqTakeCounterPtr, 0);

        for (;;) {
            if (
                TaskIndex == 1 && 
                AtomicLoadU64(ReqTakeCounterPtr, MEM_ORDER_SEQ_CST) >= Task->ThinCount / 2 &&
                AtomicLoadU32(&ASYNC_LOOP_REPEAT_HIGH_PRIORITY, MEM_ORDER_SEQ_CST)
            ) {
                AtomicExchangeU64(CancelledPtr, 1, MEM_ORDER_SEQ_CST);
            }

            if (AtomicLoadU64(CancelledPtr, MEM_ORDER_SEQ_CST))
                break;

            u64 ReqIdx = AtomicAddFetchU64(ReqTakeCounterPtr, 1, MEM_ORDER_SEQ_CST) - 1;

            if (ReqIdx >= Task->ThinCount)
                break;

            OCRequest* R = &Task->Thin[ReqIdx];
            u64 ThinLaneBroadcastMem = 0;
            LaneContext ThinLaneCtx = {0, 1, {}, &ThinLaneBroadcastMem};
            LaneContext LaneCtxRestore = SetLaneContext(ThinLaneCtx);
            b32 Retry = FALSE;
            u64 Gen = R->Generation;
            OCObject Val = R->CreateFunc(R->Key, R->CancelSignal, &Retry, &Gen);

            SetLaneContext(LaneCtxRestore);

            if (Retry && !AtomicLoadU32(R->CancelSignal, MEM_ORDER_SEQ_CST)) {
                OCRequestBatch* Batch = &OC_SHARED->RequestBatches[TaskIndex];

                LOCK_SCOPE(Batch->Mtx) {
                    OCRequestNode* N = ArenaPushArray(Batch->MemPool, OCRequestNode, 1);

                    SLL_QUEUE_PUSH(Batch->HeadThin, Batch->TailThin, N);
                    ++Batch->ThinCount;
                    MemCpy(&N->V, R, sizeof(*R));
                    N->V.Key = ArenaPushStrCpy(Batch->MemPool, N->V.Key);
                }

                AtomicExchangeU32(&ASYNC_LOOP_REPEAT, TRUE, MEM_ORDER_SEQ_CST);
            }

            OCCache* Cache = NULL;

            if (!Retry) {
                u64 CacheHash = Hash(Str8Struct(&R->CreateFunc));
                u64 CacheSlotIndex = CacheHash % OC_SHARED->CacheSlotsCount;
                Stripe* CacheStripe = StripeFromSlotIndex(
                    &OC_SHARED->CacheStripes, 
                    CacheSlotIndex
                );

                LOCK_SCOPE_R(CacheStripe->RWMtx) {
                    for (
                        OCCache* C = OC_SHARED->CacheSlots[CacheSlotIndex]; 
                        C; 
                        C = C->Next
                    ) {
                        if (C->CreateFunc == R->CreateFunc) {
                            Cache = C;
                            break;
                        }
                    }
                }
            }

            if (!Retry) {
                u64 KeyHash = Hash(R->Key);
                u64 SlotIndex = KeyHash % Cache->SlotsCount;
                OCSlot* Slot = &Cache->Slots[SlotIndex];
                Stripe* S = StripeFromSlotIndex(&Cache->Stripes, SlotIndex);

                LOCK_SCOPE_W(S->RWMtx) {
                    for (OCNode* N = Slot->Head; N; N = N->Next) {
                        if (StrMatch(N->Key, R->Key, 0)) {
                            N->LastCompletedGeneration = Gen;
                            N->V = Val;
                            AtomicSubFetchU64(&N->WorkingCount, 1, MEM_ORDER_SEQ_CST);
                            AtomicAddFetchU64(&N->CompletionCount, 1, MEM_ORDER_SEQ_CST);

                            break;
                        }
                    }
                }

                CondVarBroadcast(S->CondVar);
            }
        }

        LaneSync();
        DoneThinCount = AtomicLoadU64(ReqTakeCounterPtr, MEM_ORDER_SEQ_CST);
        LaneSync();

        if (LaneIndex() == 0 && TaskIndex > 0) {
            b32 NeedAnotherTry = (
                DoneWideCount < Task->WideCount || 
                DoneThinCount < Task->ThinCount
            );
            OCRequestBatch* Batch = &OC_SHARED->RequestBatches[TaskIndex];

            LOCK_SCOPE(Batch->Mtx) {
                for (u64 Idx = DoneWideCount; Idx < Task->WideCount; ++Idx) {
                    OCRequest* R = &Task->Wide[Idx];
                    OCRequestNode* N = ArenaPushArray(Batch->MemPool, OCRequestNode, 1);

                    SLL_QUEUE_PUSH(Batch->HeadWide, Batch->TailWide, N);
                    ++Batch->WideCount;
                    MemCpy(&N->V, R, sizeof(*R));
                    N->V.Key = ArenaPushStrCpy(Batch->MemPool, N->V.Key);
                }

                for (u64 Idx = DoneThinCount; Idx < Task->ThinCount; ++Idx) {
                    OCRequest* R = &Task->Thin[Idx];
                    OCRequestNode* N = ArenaPushArray(Batch->MemPool, OCRequestNode, 1);

                    SLL_QUEUE_PUSH(Batch->HeadThin, Batch->TailThin, N);
                    ++Batch->ThinCount;
                    MemCpy(&N->V, R, sizeof(*R));
                    N->V.Key = ArenaPushStrCpy(Batch->MemPool, N->V.Key);
                }
            }

            if (NeedAnotherTry)
                AtomicExchangeU32(&ASYNC_LOOP_REPEAT, TRUE, MEM_ORDER_SEQ_CST);
        }

        LaneSync();
    }

    LaneSync();

    if (!LaneIndex())
        SemaphoreLock(OC_SHARED->CancelThreadSemaphore, U64_MAX);

    ReleaseScratch(Scratch);
}

void 
OCCancelThreadEntryPoint(void* Params)
{
    for (;;) {
        SleepMSecs(50);
        SemaphoreLock(OC_SHARED->CancelThreadSemaphore, U64_MAX);

        for (
            u64 CacheSlotIndex = 0; 
            CacheSlotIndex < OC_SHARED->CacheSlotsCount; 
            ++CacheSlotIndex
        ) {
            Stripe* CacheStripe = StripeFromSlotIndex(&OC_SHARED->CacheStripes, CacheSlotIndex);

            LOCK_SCOPE_R(CacheStripe->RWMtx) {
                for (
                    OCCache* Cache = OC_SHARED->CacheSlots[CacheSlotIndex]; 
                    Cache; 
                    Cache = Cache->Next
                ) {
                    r1u64 SlotRange = LaneRange(Cache->SlotsCount);

                    for (
                        u64 SlotIndex = SlotRange.Min; 
                        SlotIndex < SlotRange.Max; 
                        ++SlotIndex
                    ) {
                        OCSlot* Slot = &Cache->Slots[SlotIndex];
                        Stripe* S = StripeFromSlotIndex(&Cache->Stripes, SlotIndex);

                        for (
                            b32 WriteMode = FALSE; 
                            WriteMode <= TRUE; 
                            ++WriteMode
                        ) {
                            b32 SlotHasWork = FALSE;

                            if (!WriteMode) {
                                LOCK_SCOPE_R(S->RWMtx) {
                                    for (OCNode* N = Slot->Head; N; N = N->Next) {
                                        if (
                                            AccessPointIsExpired(
                                                &N->Point, 
                                                __Params.Time = N->EvictionThresholdUSecs
                                            ) &&
                                            AtomicLoadU64(
                                                &N->WorkingCount, 
                                                MEM_ORDER_SEQ_CST
                                            ) > 0
                                        ) {
                                            SlotHasWork = TRUE;
                                            break;
                                        }
                                    }
                                }
                            } else {
                                LOCK_SCOPE_W(S->RWMtx) {
                                    for (OCNode* N = Slot->Head; N; N = N->Next) {
                                        if (
                                            AccessPointIsExpired(
                                                &N->Point, 
                                                __Params.Time = N->EvictionThresholdUSecs
                                            ) &&
                                            AtomicLoadU64(
                                                &N->WorkingCount, 
                                                MEM_ORDER_SEQ_CST
                                            ) > 0
                                        ) {
                                            SlotHasWork = TRUE;
                                            N->Cancelled = TRUE;
                                        }
                                    }
                                }
                            }

                            if (!SlotHasWork)
                                break;
                        }
                    }
                }
            }
        }

        SemaphoreUnlock(OC_SHARED->CancelThreadSemaphore);
    }
}
