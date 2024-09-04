import sys

file1 = open(sys.argv[1], 'r')
count = 0

font1 = []

while True:
    count += 1

    line = file1.readline()

    if not line:
        break

    values = line.split(', ')

    font = []

    for v in values:
        if '//' not in v:
            font.append(f'{int(v, 16):08b}')

    for f in font:
        j = list(f)
        str = ""
        for i in range(len(list(f)), 0, -1):
            j.insert(i, j[i - 1])
        font1.append(hex(int(str.join(j[:8]), 2)))
        font1.append(hex(int(str.join(j[:8]), 2)))


file1.close()

str = ""
i = 0
font2 = []
tmp = []
for f in font1:
    if (i + 1) % 14 == 0:
        tmp.append('')
    elif (i + 1) % 14 == 1:
        tmp.append('')
    elif (i + 1) % 14 == 2:
        tmp.append('{' + f + ', ')
    elif (i + 1) % 14 == 13:
        tmp.append(f + '},')
    else:
        tmp.append(f + ', ')
    i += 1
    if i % 14 == 0:
        font2.append(str.join(tmp))
        tmp = []

for f in font2:
    print(f)