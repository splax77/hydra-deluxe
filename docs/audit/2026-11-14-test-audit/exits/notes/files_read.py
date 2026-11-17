import os, sys
AUD = sys.argv[1]
N = os.path.join(AUD, 'exits', 'notes', 'batches')
READ = {
    'N01': {'capture/byreading/SUMMARY.md', 'capture/byreading/B8-verdicts.md', 'capture/rejudge/J2-R4a.md', 'capture/rejudge/J4-store-hooks.md'},
    'N02': set(),
    'N03': {'records/A35.md', 'records/A38.md', 'records/A47.md', 'records/A50.md'},
    'N04': {'records/B5.md'},
    'N05': {'records/C14.md', 'records/C15.md', 'records/C4.md'},
    'N06': {'records/D16.md'},
    'N07': None,  # all but the seven below
    'N08': set(),
    'N09': None,
}
N07_UNREAD = {'records/window-WN4c.md', 'records/window-WN5.md', 'records/window-WN8.md',
              'sweep/SW1.md', 'sweep/SW2.md', 'sweep/SW3.md', 'sweep/SW4.md'}
read, unread = [], []
for b in sorted(READ):
    files = [l.strip() for l in open(os.path.join(N, b + '-files.txt'), encoding='utf-8') if l.strip()]
    for f in files:
        if b == 'N09' or (b == 'N07' and f not in N07_UNREAD) or (READ[b] and f in READ[b]):
            read.append(f)
        else:
            unread.append(f)
    if READ[b]:
        miss = READ[b] - set(files)
        if miss:
            raise SystemExit('%s: read files not in its list: %s' % (b, miss))
if not N07_UNREAD <= set(l.strip() for l in open(os.path.join(N, 'N07-files.txt'), encoding='utf-8')):
    raise SystemExit('N07 unread names not in its list')
print(len(read), 'read whole,', len(unread), 'not reached')
with open(os.path.join(AUD, 'exits', 'notes', 'files-read.md'), 'w', encoding='utf-8', newline='\n') as fh:
    fh.write('# E5: which files a scout read whole\n\nBuilt from each first-round scout\'s "Files read" list (`N01.md` to `N09.md`). A file read whole was searched for notes the ledger script missed. A file not reached was not; its pulled notes were still each opened and marked in round two.\n\n')
    fh.write('## Read whole (%d)\n\n' % len(read) + '\n'.join('- `%s`' % f for f in sorted(read)) + '\n\n')
    fh.write('## Not reached (%d)\n\n' % len(unread) + '\n'.join('- `%s`' % f for f in sorted(unread)) + '\n')
