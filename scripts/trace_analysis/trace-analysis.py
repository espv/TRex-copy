
import sys
import re
from collections import Counter

FIRST_TRACEID = 1
#from openpyxl import Workbook
#
#wb = Workbook()
#
#ws = wb.create_sheet("Mysheet") # insert at the end (default)
#
#ws.title = "New Title"
#
#ws.sheet_properties.tabColor = "1072BA"
#
#wb.save('output/'+sys.argv[1]+'.xlsx')

from enum import Enum
from openpyxl_templates import TemplatedWorkbook
from openpyxl_templates.table_sheet import TableSheet
from openpyxl_templates.table_sheet.columns import CharColumn, ChoiceColumn, DateColumn, IntColumn
import numpy as np
import numpy_indexed as npi
from matplotlib import pyplot as plt
np.set_printoptions(edgeitems=30, linewidth=100000,
                    formatter=dict(float=lambda x: "%.3g" % x))
class Fruits(Enum):
    apple = 1
    banana = 2
    orange = 3

class TraceSheet(TableSheet):
    traceId = CharColumn()
    cpuId = CharColumn()
    threadId = CharColumn()
    timestamp = CharColumn()


class TraceIdheet(TableSheet):
    traceIdFrom = CharColumn()
    traceIdTo = CharColumn()
    cpuId = CharColumn()
    threadId = CharColumn()
    timestamp = IntColumn()


class TraceWorkBook(TemplatedWorkbook):
    regular_trace_entries = TraceSheet()
    traceid_trace_entries = TraceIdheet()


class TraceEntry():
    def __init__(self, traceId, cpuId, threadId, timestamp, cur_prev_time_diff):
        self.traceId = traceId
        self.cpuId = cpuId
        self.threadId = threadId
        self.timestamp = timestamp
        self.cur_prev_time_diff = cur_prev_time_diff


class Trace():
    def __init__(self, fn, output_fn):
        self.rows = []
        self.raw_rows = []
        self.trace = open(fn, "r")
        self.wb = TraceWorkBook()
        self.output_fn = output_fn
        self.numpy_rows = None
    
    def collect_data(self):
        previous_time = 0
        for l in self.trace:
            split_l = re.split('\t|\n', l)
            traceId = int(split_l[0])
            cpuId = int(split_l[1])
            threadId = int(split_l[2])
            t = int(split_l[3])
            if traceId == FIRST_TRACEID:
                previous_time = t
            self.rows.append(TraceEntry(traceId, threadId, cpuId, t, t-previous_time))
            previous_time = t

        self.numpy_rows = np.array([[te.traceId, te.threadId, te.cpuId, te.timestamp, te.cur_prev_time_diff] for te in self.rows])
        print(self.numpy_rows)
        print("\n")
        grouped_by_traceId = npi.group_by(self.numpy_rows[:, 0]).split(self.numpy_rows[:, :])
        for r in grouped_by_traceId:
            print(r, "\n")

        plt.title("Matplotlib demo")
        plt.xlabel("x axis caption")
        plt.ylabel("y axis caption")
        #y = [np.percentile(arr[4:], 50) for arr in grouped_by_traceId]
        #plt.plot(np.arange(len(grouped_by_traceId)),y)
        y = []
        for group in grouped_by_traceId:
            diffs = np.array([r[4] for r in group])
            y.append(diffs)

        #y = grouped_by_traceId[:, 4]
        fig, ax = plt.subplots()
        #ax.plot(np.arange(len(grouped_by_traceId)), np.asarray([np.percentile(fifty, 50) for fifty in y]), label='50th percentile')
        #ax.plot(np.arange(len(grouped_by_traceId)), np.asarray([np.percentile(fourty, 40) for fourty in y]), label='40th percentile')
        #ax.plot(np.arange(len(grouped_by_traceId)), np.asarray([np.percentile(thirty, 30) for thirty in y]), label='30th percentile')
        #ax.plot(np.arange(len(grouped_by_traceId)), np.asarray([np.percentile(twenty, 20) for twenty in y]), label='20th percentile')
        #ax.plot(np.arange(len(grouped_by_traceId)), np.asarray([np.percentile(seventy, 70) for seventy in y]), label='70th percentile')
        #ax.plot(np.arange(len(grouped_by_traceId)), np.asarray([np.percentile(eighty, 80) for eighty in y]), label='80th percentile')
        #ax.plot(np.arange(len(grouped_by_traceId)), np.asarray([np.percentile(sixty, 60) for sixty in y]), label='60th percentile')
        #ax.plot(np.arange(len(grouped_by_traceId)), np.asarray([np.percentile(ten, 10) for ten in y]), label='10th percentile')
        #ax.plot(np.arange(len(grouped_by_traceId)), np.asarray([np.percentile(one, 1) for one in y]), label='1th percentile')
        #ax.plot(np.arange(len(grouped_by_traceId)), np.asarray([np.percentile(ninety, 90) for ninety in y]), label='90th percentile')
        #ax.plot(np.arange(len(grouped_by_traceId)), np.asarray([np.percentile(ninetynine, 99) for ninetynine in y]), label='99h percentile')

        #ax.hist(y[5])
        import seaborn as sns
        for group in y:
            try:
                sns.distplot(group)
                plt.show()
            except np.linalg.linalg.LinAlgError:
                pass

    def regular_as_xlsx(self):
        self.wb.regular_trace_entries.write(
            title="Trace",
            objects=((te.traceId, te.cpuId, te.threadId, te.timestamp) for te in self.rows)
        )

        self.wb.save(self.output_fn)

    def traceid_as_xlsx(self):
        self.wb.traceid_trace_entries.write(
            title="Trace ID analytics",
            objects=()
        )

        self.wb.save(self.output_fn)


argv = sys.argv

trace = Trace(argv[1], "processed-trace.xlsx")
trace.collect_data()
trace.regular_as_xlsx()
trace.traceid_as_xlsx()

exit(0)

if len(argv) < 2:
    print("USAGE: python unique_sequences.py <file> <delimiter from> <delimiter to> <[include delimiter to]>")
    exit(0)


max_diff = 99999999
if "--max-diff" in argv:
    max_diff = int(argv[argv.index("--max-diff")+1])

receive_trace = "0"
transmit_trace = "23"

traces = open(argv[1], "r")

del1 = del2 = None
if len(argv) >= 4:
    del1 = argv[2]
    del2 = argv[3]
    print("del1:", del1)
    print("del2:", del2)

first_time = None
number_forwarded = 0
number_received = 0

include_del2 = False
if len(argv) >= 5 and argv[4] == "inc":
    include_del2 = True
    print("Including delimiter 2")
else:
    print("Not including delimiter 2")

if len(argv) >= 2:
    print("File:", argv[1])

number_between_dels = 9999999999
#if len(argv) >= 6:
#	number_between_dels = int(argv[5])

exact = False
if len(argv) >= 7 and argv[6] == 'exact':
    exact = True

last_del1 = ""
last_del1_time = 0
prev_time = 0

unique_sequences = []
cur_sequence = []

time = None
for i, l in enumerate(traces):
    time = int(l.split(" ")[0])
    if first_time is None:
        first_time = time
    eid = l.split(" ")[1]
    eid = eid[:-1]
    if eid == transmit_trace:
        number_forwarded += 1
    elif eid == receive_trace:
        number_received += 1

    if eid == del1 and (last_del1 == "" or eid != del2):
        last_del1 = eid
        last_del1_time = time
        cur_sequence = []
    elif eid == del2 and last_del1 != "" and (not exact or len(cur_sequence) == number_between_dels - 1):
        if include_del2:
            prev_time = time
            cur_sequence.append(eid)
        found = False
        for s in unique_sequences:
            if s[1] == cur_sequence:
                s[0] += 1
                if time-last_del1_time < max_diff:
                    s[2].append((i, prev_time-last_del1_time))
                found = True
                break
        if found is False and time-last_del1_time < max_diff:
            unique_sequences.append([1, cur_sequence, [(i, prev_time-last_del1_time)]])
        last_del1 = ""
        last_del1_time = 0
        cur_sequence = []
        # If del1 == del2
        if eid == del1:
            last_del1 = eid
            last_del1_time = time

    if last_del1 != "":
        cur_sequence.append(eid)

    prev_time = time
    if exact and len(cur_sequence) == number_between_dels:
        last_del1 = eid
        last_del1_time = time
        cur_sequence = []

for sequence in unique_sequences:
    print("Occurrences:", sequence[0], "sequence:", ", ".join(sequence[1]))
    print("Occurrences with line numbers:", sequence[2], "\n")
    l = sorted([r[1] for r in sequence[2]])
    print("All unique times and number of occurrences:", Counter(l).most_common())
    print("Max:", max(l), ", min:", min(l))
    print("Avg:", sum(l)/float(len(l)), ", median:", l[len(l)/2], "\n")

if time is None or first_time is None or number_forwarded == 0:
    sys.exit(0)
print("Number forwarded:", number_forwarded, "first_time:", first_time, "- last_time:", time)
print("Forwarded on avg every", (time-first_time)/number_forwarded)
print("Number received:", number_received, "- on avg every", (time-first_time)/number_received, "\n")