"""One-off repair: 15 count rows whose test cell begins with a legend code were mapped to a file named
in another column. Sets their test_file from the legend and moves them in chunks.tsv.
Usage: py -I fix_legend_rows.py   (run in exits/)"""
import csv

NEW = {
    **{'counts/T5-IR4.md:%d' % n: ('~/.claude/gates/tests/test_ref_judge.py', 'legend-base')
       for n in (61, 64, 65, 66, 67, 70, 71, 75, 76, 78, 80, 81)},
    'counts/T5-IR7.md:89': ('tools/ch_probe/tests/test_play_chart.py', 'legend-base'),
    'counts/T5-KR1.md:29': ('tools/ch_mcp/tests/test_mono_facts.py', 'legend-base'),
    'counts/T5-KR1.md:98': ('tools/ch_mcp/tests/test_server.py', 'legend-base'),
}

def read(p):
    with open(p, encoding='utf-8', newline='') as fh:
        return list(csv.DictReader(fh, delimiter='\t'))

def write(p, fields, rows):
    with open(p, 'w', encoding='utf-8', newline='') as fh:
        w = csv.DictWriter(fh, fields, delimiter='\t', lineterminator='\n')
        w.writeheader()
        w.writerows(rows)

rows = read('row-files.tsv')
for r in rows:
    if r['row'] in NEW:
        r['test_file'], r['how'] = NEW[r['row']]
write('row-files.tsv', ['batch', 'kind', 'row', 'keys', 'test_file', 'how'], rows)

chunks = read('chunks.tsv')
by = {c['chunk']: c for c in chunks}

def take(cid, ids):
    c = by[cid]
    left = [x for x in c['row_ids'].split() if x not in ids]
    c['row_ids'] = ' '.join(left)
    c['rows'] = str(len(left))

def give(cid, ids):
    c = by[cid]
    c['row_ids'] = ' '.join(c['row_ids'].split() + ids)
    c['rows'] = str(len(c['row_ids'].split()))

ir4 = [k for k in NEW if 'IR4' in k]
take('C285', set(ir4))
c = by['C285']
c.update(kind='file-part', part='4/4', files='~/.claude/gates/tests/test_ref_judge.py')
give('C285', ir4)
for cid, p in (('C103', '1/4'), ('C104', '2/4'), ('C105', '3/4')):
    by[cid]['part'] = p
take('C287', {'counts/T5-KR1.md:29', 'counts/T5-KR1.md:98'})
give('C221', ['counts/T5-KR1.md:29'])
take('C286', {'counts/T5-IR7.md:89'})
# The two rows of files whose chunks are all merged go in one new chunk.
by['C286'].update(kind='files', part='1/1', files='tools/ch_mcp/tests/test_server.py tools/ch_probe/tests/test_play_chart.py',
                  row_ids='counts/T5-IR7.md:89 counts/T5-KR1.md:98', rows='2')
chunks = [c for c in chunks if c['row_ids'].strip()]
write('chunks.tsv', list(chunks[0].keys()), chunks)
