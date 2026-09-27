# FUME

FUME examines MAVLink telemetry logs (`.tlog` files). It tests the checksum of each packet, finds the bytes that are not in a record, finds the cause of each incorrect packet, and shows where in the file the damage is.

Use FUME when the data in your ground control station (GCS) is not correct and you want to know why. Some examples:

- The GCS shows a vehicle with a system ID that you do not have.
- You think that a radio link or a serial link changes bytes.
- You changed a message definition in your firmware and you are not sure that the GCS has the same definition.
- You have many logs and you want to find the flight where the problem started.
- You want to know if the radio link, the clock of the vehicle or the clock of the GCS caused a problem.

FUME decodes the payload of four messages only: `HEARTBEAT`, `SYS_STATUS`, `SYSTEM_TIME` and `RADIO_STATUS`. FUME does not show payload bytes. FUME does not change the log files. FUME opens each file for read access only.

## Contents

- [Usage](#usage)
- [How to read the report](#how-to-read-the-report)
- [Message IDs that are not in the table](#message-ids-that-are-not-in-the-table)
- [Limits](#limits)

## Usage

```
fume [options] <file.tlog | directory> ...
```

You can give FUME files, directories, or the two together. For a directory, FUME reads each file that has the extension `.tlog`. Upper case and lower case are the same. FUME does not go into subdirectories. A file that you give directly can have any extension.

| Option | Function |
| --- | --- |
| `--threads=N` | The number of threads. The range is 1 to 256. The default is the number of logical processors. |
| `--bad=N` | The number of problems that the report lists for each file, and the number of incorrect packets in the `LOST PACKETS` list. The default is 16. `--bad=0` removes the lists. |
| `--msgs` | Adds a table of the packets and the bytes for each message. |
| `--dump` | Adds one line for each packet, in the sequence that the packets have in the file. FUME does not show payloads. |
| `--sys=A,B` | Counts only the packets that have one of these system IDs. |
| `--comp=A,B` | Counts only the packets that have one of these component IDs. |
| `--msg=A,B` | Counts only the packets that have one of these message IDs or message names. |
| `--time=A,B` | Counts only the packets from A seconds to B seconds after the first timestamp of the file. B is optional. |
| `--offset=A,B` | Examines only the bytes from offset A to offset B. The offsets are hexadecimal. B is optional. |
| `--extract=F` | Writes the bytes in `--offset` to the file F. Use with one input file only. |
| `--no-colour` | Removes the colour codes from the report. |
| `--help`, `--h`, `--?` | Shows the usage message. |

A problem is a packet with an incorrect checksum, or a block of bytes that are not in a record.

The options `--sys`, `--comp`, `--msg` and `--time` are filters. A packet that does not pass a filter is not counted, is not in the tables and is not in the `--dump` list. The filters do not apply to bytes that are not in a record. FUME always counts these bytes. The report shows the filters that are in use on the `Filter` line below the file name.

The message names for `--msg` are the names in `MAVLINK_MSG_XLIST`. Upper case and lower case are the same.

### Example Usages

Examine one log:

```
fume flight_07.tlog
```

Examine all logs in a directory and keep the report:

```
fume --no-colour logs/ > report.txt
```

Show the message table and the first 100 problems:

```
fume --msgs --bad=100 flight_07.tlog
```

Find the incorrect packets in the packet list:

```
fume --dump --no-colour flight_07.tlog | grep Incorrect
```

Count only the packets from the autopilot, in the first five minutes of the flight:

```
fume --sys=1 --comp=1 --time=0,300 flight_07.tlog
```

Count only the `HEARTBEAT` and `RADIO_STATUS` packets:

```
fume --msg=HEARTBEAT,109 flight_07.tlog
```

Cut the bytes of one damaged segment out of a log and put them in a small file:

```
fume --offset=1930000,1940000 --extract=segment.tlog flight_07.tlog
```

### Notes

- The report uses ANSI colour codes and UTF-8 line characters. Use `--no-colour` when you send the report to a file.
- FUME prints the report for a file first. The `--dump` list for that file comes after the report.
- The last line of the output shows the number of files, the total size, the total time, the time in the packet walk only (`WALK`), the number of threads and the rate. The time that is not in `WALK` is the time to open the files and to write the report.

## How to read the report

### The file header

The file name, the size and the first and last timestamp in the file. If a filter is in use, the `Filter` line shows it.

### PACKETS

The total number of packets, and the number for each MAVLink version. FUME counts the packets that have a signature. FUME does not test the signature.

`Correct, a copy of the previous packet` is the number of correct packets that are the same, byte for byte, as the previous packet from the same system ID and component ID. A link or a GCS that sends a packet two times causes these.

### CHECKSUM

| Result | Meaning |
| --- | --- |
| Correct | The checksum in the packet agrees with the checksum that FUME calculated. |
| Incorrect | The checksums do not agree. One byte or more changed after the sender calculated the checksum. |
| Not tested | The message ID is not in the FUME message table. FUME cannot test the checksum without the `CRC_EXTRA` value of the message. See [Message IDs that are not in the table](#message-ids-that-are-not-in-the-table). |

### BYTES THAT ARE NOT IN A RECORD

These are bytes between records that FUME cannot read as a record. A block of these bytes can be a record with an incorrect length byte, a part of a record, or data that is not MAVLink. `Blocks` is the number of such blocks. `Bytes` is the total size of the blocks. `Blocks that are text` is the number of blocks that contain printable text only. Text in a log usually comes from a device on the serial link that is not a MAVLink device, for example a bootloader or a modem in command mode.

### SYSTEM ID AND COMPONENT ID

One row for each pair of system ID and component ID, in the sequence of the system ID and then the component ID. A pair gets a row only if it has one correct packet or more. `COPIES` is the number of correct packets from the pair that are a copy of the previous packet.

When a system ID byte or a component ID byte changes in the link, the result is a new pair that has no correct packet. A log with link damage can contain many such pairs. FUME does not give them rows. FUME adds their packets together and shows the total below the table. Use `--dump` to see each of these packets.

If your GCS shows a system ID that you do not have, look at this table first:

- The ID has a row. A packet with that ID and a correct checksum is in the log. The sender calculated the checksum with that ID. Link damage is not the probable cause.
- The ID has no row. All packets with that ID have an incorrect checksum or FUME did not test them. Link damage is the probable cause.

### ALL PACKETS FROM EACH SYSTEM ID AND COMPONENT ID THAT HAS 8 PACKETS OR LESS

A pair that has a row but very few packets is unusual. FUME lists all packets from such a pair, so that you can see what they are and when they arrived. This section is not in the report if there are no such pairs.

### HEARTBEAT FROM EACH SYSTEM ID AND COMPONENT ID

One row for each pair that sent a correct `HEARTBEAT` packet. This section is not in the report if there is no such pair.

| Column | Meaning |
| --- | --- |
| `HEARTBEAT` | The number of correct `HEARTBEAT` packets. |
| `SECONDS` | The time from the first to the last `HEARTBEAT`. An autopilot sends one `HEARTBEAT` each second. Compare the two columns to see how many heartbeats did not arrive. |
| `TYPE`, `AUTOPILOT` | The `type` and `autopilot` fields of the first `HEARTBEAT`. |
| `CHANGES` | The number of `HEARTBEAT` packets that have a type or autopilot that is not the same as in the `HEARTBEAT` before it. A pair that changes type is possibly two devices that use the same IDs. |

### SYS_STATUS AND SYSTEM_TIME FROM EACH SYSTEM ID AND COMPONENT ID

One row for each pair that sent a correct `SYS_STATUS` or `SYSTEM_TIME` packet. This section is not in the report if there is no such pair.

| Column | Meaning |
| --- | --- |
| `SYS_STATUS` | The number of correct `SYS_STATUS` packets. |
| `ERRORS_COMM` | The increase of the `errors_comm` field from the first to the last `SYS_STATUS`. This is the number of MAVLink errors that the autopilot counted on its own receive side. A large number shows damage on the link from the GCS to the vehicle. |
| `DROP_RATE_COMM` | The largest `drop_rate_comm` field, in percent. |
| `SYSTEM_TIME` | The number of correct `SYSTEM_TIME` packets. |
| `CLOCK OFFSET MIN`, `CLOCK OFFSET MAX` | The log timestamp minus the `time_unix_usec` field, in milliseconds. The offset is the delay from the vehicle to the log plus the difference of the two clocks. A large range between the minimum and the maximum shows that the clock of the GCS or the clock of the vehicle moved during the flight. |

### RADIO_STATUS FROM EACH SYSTEM ID AND COMPONENT ID

One row for each pair that sent a correct `RADIO_STATUS` packet. This section is not in the report if there is no such pair. The radio modem is usually system ID 51, component ID 68.

| Column | Meaning |
| --- | --- |
| `RADIO_STATUS` | The number of correct `RADIO_STATUS` packets. |
| `RSSI MIN`, `NOISE MAX` | The lowest `rssi` field and the highest `noise` field. |
| `RXERRORS`, `FIXED` | The increase of the `rxerrors` and `fixed` fields from the first to the last `RADIO_STATUS`. `RXERRORS` is the number of radio packets with errors. `FIXED` is the number of these that the radio corrected. |

### INCORRECT CHECKSUMS AND BYTES THAT ARE NOT IN A RECORD

The first problems in the file, in file sequence. `--bad=N` sets the number of lines. The `OFFSET` column is the byte offset of the record in the file. Use it to find the record in a hex editor.

For each incorrect packet, FUME calculates the one change that makes the checksum correct. The `CHECKSUM` column shows the result:

| Text | Meaning |
| --- | --- |
| `Incorrect. Correct if the system ID is 1.` | One header byte is incorrect. FUME shows the field and the value that makes the checksum correct. |
| `Incorrect. One payload byte is incorrect.` | One payload byte is incorrect. FUME does not show payload values. |
| `Incorrect. Correct if the byte 0x3A is put in at index 12.` | The link lost one byte. FUME shows the byte and the position where it was. |
| `Incorrect. Correct if the byte at index 12 is removed.` | The link added one byte. FUME shows its position. |
| `Incorrect. Correct if the length is 28.` | The length byte is incorrect. The block of bytes is a correct packet with the length that FUME shows. |
| `Incorrect` | More than one change is necessary. |
| `Not tested. Correct if CRC_EXTRA is 117.` | The message ID is not in the table. FUME shows the `CRC_EXTRA` value that makes the checksum correct. |

The index is the position of the byte from the start of the packet. The magic byte is at index 0.

For a block of bytes that are not in a record, FUME shows what the block is:

| Text | Meaning |
| --- | --- |
| `The first bytes are a packet:` | The block starts with a MAVLink header. The next line shows that packet and the change that makes it correct. |
| `The first bytes are a packet that the end of the file cuts off.` | The block starts with a MAVLink header, but the file ends before the packet ends. |
| `The bytes are text: "..."` | The block contains printable text only. FUME shows the first 60 characters. |

This result is a calculation. It is not a proof. The checksum has 16 bits, so a packet with many incorrect bytes can, by chance, look like a packet with one incorrect byte. This is rare, less than 1 in 200 for each packet.

### LOST PACKETS

When a length byte is incorrect and too large, the GCS reads too many bytes as one packet. The bytes of the correct packets that follow are then inside the incorrect packet. The GCS did not decode these packets and did not write a timestamp for them. They are not in the tables above, and they were not on the screen of the GCS.

FUME finds these packets. It examines each byte inside an incorrect packet, and inside each block of bytes that are not in a record. A byte is the start of a correct packet if it has a MAVLink magic value, the length after it fits, the message ID is in the table, and the checksum is correct. FUME calculates the checksum, so a chance result is rare: approximately 1 in 10 million bytes.

| Line | Meaning |
| --- | --- |
| Correct packets that are in incorrect packets | The number of correct packets that FUME found inside incorrect packets and inside blocks of bytes that are not in a record. |
| Packets that are cut off at the end of these | After the last correct packet, some bytes can remain before the end of the incorrect packet. If they start with a magic value, they are the start of one more packet. The GCS did not write the end of that packet. FUME counts one cut off packet for each such incorrect packet. |
| Incorrect packets that have correct packets in them | The number of incorrect packets and blocks that contain one correct packet or more. |

Below the counts is a list. `--bad=N` sets the number of incorrect packets in the list. Each incorrect packet is one red line. The correct packets inside it are grey lines below it. If a packet is cut off, the last line shows the number of its bytes that are in the file.

The correct packets in the list have `No timestamp` in the `TIMESTAMP` column, because the GCS did not write one. The `OFFSET` column of these lines is the offset of the magic byte of the packet. It is not the offset of a timestamp.

```
    0x1938BAB    2026-09-24 01:16:02.243187    133         1        0  Not in table (41473)     253  Not tested. Correct if CRC_EXTRA is 0.
    0x1938BB4    No timestamp                    1         1      133  FENCE_STATUS (162)         1  Correct
    0x1938BC1    No timestamp                    1         1      134  WIND (168)                 8  Correct
    0x1938BD5    No timestamp                    1         1      135  RANGEFINDER (173)          4  Correct
    0x1938CA2    39 bytes of a packet that is cut off
```

How to read the list:

- The sequence numbers of the correct packets are consecutive. This shows that they are real packets from one sender.
- The header of the incorrect packet is usually made of the bytes of the first correct packet, moved by one byte or more. In the example, the length 253 is `0xFD`, the magic byte of the first correct packet, and the system ID 133 is its sequence number. Here the link added one byte before a correct packet. The GCS read the added byte as a magic byte.
- An incorrect packet can also be the end of a packet that lost its start. Its header then contains the last fields of that packet, for example the callsign of an `ADSB_VEHICLE` message. The packet that ate the start of that packet is usually the incorrect packet before it. In such a chain, one damage event causes more than one incorrect packet.

This section is not in the report if FUME found no packet inside an incorrect packet.

### CAUSE OF THE INCORRECT PACKETS AND THE BYTES THAT ARE NOT IN A RECORD

The same calculation for all incorrect packets and all blocks in the file, as totals. This section is not in the report if all checksums are correct and there are no blocks.

| Line | Meaning |
| --- | --- |
| One byte is incorrect: the length, the sequence, the system ID, ... | One byte changed. One line for each header field, and one for the payload. |
| One byte is lost | The link lost one byte. |
| One byte is added | The link added one byte. |
| More than one change is necessary | FUME did not find one change that makes the packet correct. |
| The bytes do not start with a packet | The block has no MAVLink header at its start. |

If most packets have one incorrect byte, the link changes single bytes. A serial link with a bad ground or a wrong baud rate does this. If most packets lose or add bytes, or need more than one change, the link loses blocks of bytes. A radio link that drops frames or a serial buffer that overflows does this.

### TIMESTAMPS

The GCS writes the timestamps from the clock of its computer. They show when the GCS received each packet.

| Line | Meaning |
| --- | --- |
| Intervals longer than 1 s | The number of times that no packet arrived for more than 1 second. |
| Incorrect packets after these intervals | The number of these intervals where the first packet after the interval is incorrect. A large number shows that the damage occurs when the link comes back. |
| Longest interval, in seconds | The longest time with no packet. |
| Earlier than the previous timestamp | The clock of the computer went back, or the file contains more than one log. |
| Not between 2010 and 2050 | The timestamp bytes are corrupt. FUME shows such a timestamp as a hexadecimal value. |

### THE FILE IN PARTS OF EQUAL SIZE

FUME divides the file into 16 parts that have the same number of bytes. Each row shows the first timestamp in the part and the counts for the part. Use this table to see if the damage is in all of the flight or in one segment. A part that contains no packet shows `No timestamp`.

### MESSAGES

Only with `--msgs`. One row for each message in the table that has a packet in the log. `BYTES` is the total size of the packets of that message, without the timestamps. Use it to see which messages use the link.

If a message has more than two packets and none of them is correct, FUME shows a note. The usual cause is a `CRC_EXTRA` value in `MAVLINK_MSG_XLIST` that is different from the value in your firmware. This occurs when you change the fields of a message in your firmware.

### ALL PACKETS IN THE FILE

Only with `--dump`. One line for each packet and one line for each block of bytes that are not in a record. The columns are the offset, the timestamp, the MAVLink version, the system ID, the component ID, the sequence, the message ID, the message name, the payload length and the checksum result. A packet with a signature has `, signature` after the checksum result.

The columns have the same width on all lines, and the lines have no colour codes. Thus you can use `grep`, `awk` or `findstr` on them.

### ALL FILES

Only when FUME reads more than one file. One row for each file, at the end of the output. Use this table to compare flights. The last column shows the system IDs that have a correct packet in that file.

## Message IDs that are not in the table

The MAVLink checksum includes one byte that is not in the packet. This byte is the `CRC_EXTRA` value. It comes from the message definition. If a message ID is not in the FUME table, FUME does not know this value and cannot test the checksum.

For these packets, FUME calculates the `CRC_EXTRA` value that makes the checksum correct. The report shows the result for each message ID:

```
  MESSAGE IDS THAT ARE NOT IN THE TABLE
    MESSAGE ID        PACKETS   CRC_EXTRA
         11045          4,210   117 (the same in all packets)
           199              1   42 (from one packet only, possibly corrupt)
         41473             13   The packets do not agree. These packets are possibly corrupt.
```

A message ID with many packets that all give the same value is a real message. A message ID with one or two packets, or with packets that do not agree, is possibly a corrupt packet. For a corrupt packet, there is approximately 1 chance in 256 that a `CRC_EXTRA` value exists.

To add a message, put one line in `MAVLINK_MSG_XLIST` in `mavlink.h` and build FUME again:

```c
X(11045, 117, MY_NEW_MESSAGE)                             \
```

The fields are the message ID, the `CRC_EXTRA` value and the name. Keep the list in the sequence of the message IDs. The `MESSAGES` table uses the sequence of the list.

The table contains the messages of the ArduPilot dialect (`ardupilotmega.xml` and the files that it includes). The message ID must be less than 65536.

## Limits

- FUME decodes the payload of `HEARTBEAT`, `SYS_STATUS`, `SYSTEM_TIME` and `RADIO_STATUS` only. FUME does not test signatures.
- A file must be 16 bytes or larger.
- The `SYSTEM ID AND COMPONENT ID` table has 64 rows. FUME tells you how many packets did not go into the table. Note that pairs with no correct packet also use rows.
- The `MESSAGE IDS THAT ARE NOT IN THE TABLE` list has 16 rows. FUME does not list more message IDs than that.
- `--extract` works with one input file only.
- FUME does not read subdirectories.
