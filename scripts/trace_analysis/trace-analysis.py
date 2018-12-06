
import sys
from collections import Counter

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

from datetime import date
from enum import Enum
from openpyxl_templates import TemplatedWorkbook
from openpyxl_templates.table_sheet import TableSheet
from openpyxl_templates.table_sheet.columns import CharColumn, ChoiceColumn, DateColumn
class Fruits(Enum):
    apple = 1
    banana = 2
    orange = 3

class PersonSheet(TableSheet):
    first_name = CharColumn()
    last_name = CharColumn()
    date_of_birth = DateColumn()
    favorite_fruit = ChoiceColumn(choices=(
        (Fruits.apple, "Apple"),
        (Fruits.banana, "Banana"),
        (Fruits.orange, "Orange"),
    ))
class PersonsWorkbook(TemplatedWorkbook):
    persons = PersonSheet()


wb = PersonsWorkbook()
wb.persons.write(
    title="List of fruit lovers",
    objects=(
        ("John", "Doe", date(year=1992, month=7, day=17), Fruits.banana),
        ("Jane", "Doe", date(year=1986, month=3, day=2), Fruits.apple),
    )
)
wb.save("my_excel.xlsx")


argv = sys.argv
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