"""Quality metrics for the generated markdown."""
import json, re
from pathlib import Path
from collections import Counter

doc = Path('output/Tricks of the 3D Game Programming Gurus')
md = (doc / 'Tricks of the 3D Game Programming Gurus.md').read_text(encoding='utf-8')
meta = json.loads((doc / 'Tricks of the 3D Game Programming Gurus_meta.json').read_text(encoding='utf-8'))

lines = md.splitlines()
words = re.findall(r'\b\w+\b', md)
print('== Size ==')
print(f'  bytes:        {len(md.encode("utf-8")):>10,}')
print(f'  chars:        {len(md):>10,}')
print(f'  lines:        {len(lines):>10,}')
print(f'  words:        {len(words):>10,}')
print(f'  blank lines:  {sum(1 for l in lines if not l.strip()):>10,}')

ps = meta.get('page_stats', [])
print('\n== Pages ==')
print(f'  page_stats entries: {len(ps)}')
ref_pages = sorted({int(m.group(1)) for m in re.finditer(r'_page_(\d+)_', md)})
if ref_pages:
    print(f'  image refs span: page {min(ref_pages)} .. {max(ref_pages)} ({len(ref_pages)} unique pages w/ figures)')

heads = Counter()
for l in lines:
    m = re.match(r'^(#{1,6})\s', l)
    if m:
        heads[len(m.group(1))] += 1
print('\n== Headings ==')
for lvl in sorted(heads):
    print(f'  H{lvl}: {heads[lvl]:>6,}')

sep_rows  = sum(1 for l in lines if re.match(r'^\s*\|[-:\s|]+\|\s*$', l))
table_lines = sum(1 for l in lines if re.match(r'^\s*\|', l))
print('\n== Tables ==')
print(f'  tables (~separator-row count): {sep_rows:,}')
print(f'  total pipe-table lines:        {table_lines:,}')
empty_cells = sum(1 for l in lines for c in re.findall(r'\|([^|\n]*)', l) if c.strip() == '')
print(f'  empty table cells:             {empty_cells:,}')

img_count = len(re.findall(r'!\[[^\]]*\]\([^)]+\)', md))
img_alt   = len(re.findall(r'!\[[^\]]+\]\([^)]+\)', md))
print('\n== Images ==')
print(f'  total image refs:   {img_count:,}')
print(f'  with non-empty alt: {img_alt:,}  ({100*img_alt/max(img_count,1):.1f}%)')

fence_count = md.count('```')
print('\n== Code ==')
print(f'  triple-fence markers: {fence_count}  (~{fence_count//2} blocks)')

inline_math = len(re.findall(r'(?<!\$)\$(?!\$)[^\$\n]+\$(?!\$)', md))
block_math  = len(re.findall(r'\$\$[\s\S]+?\$\$', md))
print('\n== Math ==')
print(f'  inline $..$:    {inline_math:,}')
print(f'  block  $$..$$:  {block_math:,}')

bold   = len(re.findall(r'\*\*[^*\n]+\*\*', md))
italic = len(re.findall(r'(?<!\*)\*[^*\n]+\*(?!\*)', md))
print('\n== Inline formatting ==')
print(f'  **bold**:  {bold:,}')
print(f'  *italic*:  {italic:,}')

print('\n== HTML noise that survived ==')
for tag in ['<sub>', '<sup>', '<b>', '<u>', '<i>', '<br>', '<table', '<tr>', '<td']:
    c = md.lower().count(tag)
    if c:
        print(f'  {tag}: {c:,}')

print('\n== Metadata ==')
print(f'  table_of_contents entries: {len(meta.get("table_of_contents", [])):,}')
if ps:
    print(f'  page_stats keys: {list(ps[0].keys())}')
    for k in ps[0]:
        vals = [p.get(k) for p in ps if isinstance(p.get(k), (int, float))]
        if vals:
            print(f'    {k:<25} total={sum(vals):>10,}   avg/page={sum(vals)/len(vals):.2f}')
