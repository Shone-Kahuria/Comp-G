import re
from docx import Document
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from docx.shared import Inches, Pt, RGBColor

md = open('Lab4_Report.md', encoding='utf-8').read().splitlines()
d = Document()
d.sections[0].page_width = Inches(8.5)
d.sections[0].page_height = Inches(11)
d.sections[0].top_margin = Inches(0.65)
d.sections[0].bottom_margin = Inches(0.65)
d.sections[0].left_margin = Inches(0.75)
d.sections[0].right_margin = Inches(0.75)

d.styles['Normal'].font.name = 'Aptos'
d.styles['Normal'].font.size = Pt(10.5)
d.styles['Normal'].paragraph_format.space_after = Pt(5)
for style_name, size in [('Title', 22), ('Heading 1', 15), ('Heading 2', 12)]:
    style = d.styles[style_name]
    style.font.name = 'Aptos Display'
    style.font.size = Pt(size)
    style.font.color.rgb = RGBColor(0, 0, 0)
    style.paragraph_format.keep_with_next = True
    p_pr = style.element.get_or_add_pPr()
    p_bdr = p_pr.find(qn('w:pBdr'))
    if p_bdr is not None:
        p_pr.remove(p_bdr)

def set_repeat_table_header(row):
    tr_pr = row._tr.get_or_add_trPr()
    tbl_header = OxmlElement('w:tblHeader')
    tbl_header.set(qn('w:val'), 'true')
    tr_pr.append(tbl_header)

def shade_cell(cell, fill):
    tc_pr = cell._tc.get_or_add_tcPr()
    shd = OxmlElement('w:shd')
    shd.set(qn('w:fill'), fill)
    tc_pr.append(shd)

def set_cell_margins(cell, top=80, start=100, bottom=80, end=100):
    tc = cell._tc
    tc_pr = tc.get_or_add_tcPr()
    tc_mar = tc_pr.first_child_found_in('w:tcMar')
    if tc_mar is None:
        tc_mar = OxmlElement('w:tcMar')
        tc_pr.append(tc_mar)
    for margin, value in [('top', top), ('start', start), ('bottom', bottom), ('end', end)]:
        node = tc_mar.find(qn(f'w:{margin}'))
        if node is None:
            node = OxmlElement(f'w:{margin}')
            tc_mar.append(node)
        node.set(qn('w:w'), str(value))
        node.set(qn('w:type'), 'dxa')

def runs(p, text):
    for i, part in enumerate(re.split(r'(\*\*[^*]+\*\*|`[^`]+`)', text)):
        if part.startswith('**'):
            p.add_run(part[2:-2]).bold = True
        elif part.startswith('`'):
            r = p.add_run(part[1:-1]); r.font.name = 'Consolas'
        else:
            p.add_run(part)

i = 0
while i < len(md):
    l = md[i]
    if l.startswith('# '):
        title = d.add_paragraph(style='Title')
        title.alignment = WD_ALIGN_PARAGRAPH.CENTER
        title.add_run(l[2:])
    elif l.startswith('## '):
        d.add_heading(l[3:], 1)
    elif l.startswith('|'):
        rows = []
        while i < len(md) and md[i].startswith('|'):
            if not re.match(r'^\|[-| ]+\|$', md[i]):
                rows.append([c.strip() for c in md[i].strip('|').split('|')])
            i += 1
        t = d.add_table(rows=len(rows), cols=len(rows[0])); t.style = 'Table Grid'
        t.autofit = True
        for r, row in enumerate(rows):
            for c, v in enumerate(row):
                cell = t.cell(r, c)
                cell.text = v.replace('`', '')
                set_cell_margins(cell)
                for paragraph in cell.paragraphs:
                    paragraph.paragraph_format.space_after = Pt(0)
                    for run in paragraph.runs:
                        run.font.size = Pt(9.5)
                if r == 0:
                    shade_cell(cell, 'D9E2F3')
                    for run in cell.paragraphs[0].runs:
                        run.bold = True
        set_repeat_table_header(t.rows[0])
        d.add_paragraph()
        continue
    elif l.startswith('- '):
        runs(d.add_paragraph(style='List Bullet'), l[2:])
    elif l.strip():
        runs(d.add_paragraph(), l)
    i += 1

d.add_heading('Screenshots', 1)
intro = d.add_paragraph('OpenGL window captures include the GL_POLYGON pentagon and triangle plus the four pixel-filling modes. Press t or p to switch shape and c to cycle the RGB palette.')
intro.paragraph_format.keep_with_next = True
window_figures = [
    ('window_mode0.png', 'Figure 1. GL_POLYGON pentagon - blue'),
    ('window_triangle.png', 'Figure 2. GL_POLYGON triangle - blue'),
    ('window_mode1.png', 'Figure 3. Boundary Fill 4-connected - red'),
    ('window_mode2.png', 'Figure 4. Boundary Fill 8-connected - green'),
    ('window_mode3.png', 'Figure 5. Flood Fill - orange'),
    ('window_mode4.png', 'Figure 6. Scan-Line Fill - purple'),
]
for filename, label in window_figures:
    caption = d.add_paragraph(label)
    caption.paragraph_format.keep_with_next = True
    caption.runs[0].bold = True
    picture = d.add_paragraph()
    picture.alignment = WD_ALIGN_PARAGRAPH.CENTER
    picture.add_run().add_picture(f'screenshots/{filename}', width=Inches(2.35))

intro = d.add_paragraph('Triangle benchmark captures use the same algorithms and a different palette colour for each run.')
intro.paragraph_format.keep_with_next = True
triangle_labels = [
    'Figure 7. Triangle - Boundary Fill 4-connected',
    'Figure 8. Triangle - Boundary Fill 8-connected',
    'Figure 9. Triangle - Flood Fill',
    'Figure 10. Triangle - Scan-Line Fill',
]
for n, label in enumerate(triangle_labels):
    caption = d.add_paragraph(label)
    caption.paragraph_format.keep_with_next = True
    caption.runs[0].bold = True
    picture = d.add_paragraph()
    picture.alignment = WD_ALIGN_PARAGRAPH.CENTER
    picture.add_run().add_picture(f'screenshots/triangle_{n}.png', width=Inches(2.2))

caption = d.add_paragraph('Figure 11. Flood Fill stops at the red divider in a multi-colour region')
caption.paragraph_format.keep_with_next = True
caption.runs[0].bold = True
picture = d.add_paragraph()
picture.alignment = WD_ALIGN_PARAGRAPH.CENTER
picture.add_run().add_picture('screenshots/flood_multicolour.png', width=Inches(2.35))

d.add_heading('Submission files', 1)
for item in [
    'AreaFill.cpp - source code for all algorithms and the OpenGL interface',
    'AreaFillGL.exe and freeglut.dll - interactive OpenGL program',
    'AreaFill.exe - headless benchmark and screenshot generator',
    'screenshots/ - PNG output evidence',
    'results.txt - recorded benchmark measurements',
]:
    d.add_paragraph(item, style='List Bullet')

d.save('Lab4_Report.docx')
print('ok')
