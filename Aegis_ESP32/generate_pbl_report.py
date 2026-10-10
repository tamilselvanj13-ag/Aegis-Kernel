# -*- coding: utf-8 -*-
"""
AEGIS KERNEL SENTINEL - PROJECT BASED LEARNING (PBL) REPORT GENERATOR
Formats the complete university PBL report matching the Neo Dimetrix template:
- Institution: Chennai Institute of Technology (Autonomous)
- Affiliation: Anna University, Chennai
- Department: Electronics and Communication Engineering
- Course: Microprocessor and Microcontrollers
- Team: Sai Prasath P (210425106193) & Tamil Selvan J (210425106242)
- Supervisor: Mr. Baskar M.E., Professor, Dept. of ECE
- HOD: Dr. T. S. Arulananth M.E., Ph.D., Professor & HOD, Dept. of ECE
- Principal: Dr. A. Ramesh M.E., Ph.D.
- Chairman: Shri. P. Sriram
"""

import os
import subprocess
import docx
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT, WD_ALIGN_VERTICAL
from docx.oxml import OxmlElement, parse_xml
from docx.oxml.ns import nsdecls, qn

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DOCX_OUT = os.path.join(BASE_DIR, "Aegis_PBL_Report.docx")
HTML_OUT = os.path.join(BASE_DIR, "Aegis_PBL_Report.html")
PDF_OUT = os.path.join(BASE_DIR, "Aegis_PBL_Report.pdf")

DOWNLOADS_DOCX = os.path.join(r"C:\Users\Tamil Selvan J\Downloads", "Aegis_PBL_Report.docx")
DOWNLOADS_PDF = os.path.join(r"C:\Users\Tamil Selvan J\Downloads", "Aegis_PBL_Report.pdf")

def set_cell_background(cell, fill_hex):
    shading_elm = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{fill_hex}"/>')
    cell._tc.get_or_add_tcPr().append(shading_elm)

def set_cell_margins(cell, top=100, bottom=100, left=150, right=150):
    tcPr = cell._tc.get_or_add_tcPr()
    tcMar = OxmlElement('w:tcMar')
    for m, val in [('top', top), ('bottom', bottom), ('left', left), ('right', right)]:
        node = OxmlElement(f'w:{m}')
        node.set(qn('w:w'), str(val))
        node.set(qn('w:type'), 'dxa')
        tcMar.append(node)
    tcPr.append(tcMar)

def set_table_borders(table, color="CCCCCC", sz="4"):
    tblPr = table._tbl.tblPr
    borders = parse_xml(f'''
        <w:tblBorders {nsdecls("w")}>
            <w:top w:val="single" w:sz="{sz}" w:space="0" w:color="{color}"/>
            <w:bottom w:val="single" w:sz="{sz}" w:space="0" w:color="{color}"/>
            <w:insideH w:val="single" w:sz="{sz}" w:space="0" w:color="{color}"/>
            <w:insideV w:val="none"/>
            <w:left w:val="none"/>
            <w:right w:val="none"/>
        </w:tblBorders>
    ''')
    tblPr.append(borders)

print("[*] Script initialized successfully.")
