from __future__ import annotations

from typing import Dict, List, Set

PROVIDER_REGISTRY: Dict[str, Set[str]] = {
    "system": {"probe"},
    "smoke": {"ping"},
    "mammoth": {"docx_to_html"},
    "mammoth_semantic": {"export_html", "extract_raw_text"},
    "docxcompose": {"compose_append"},
    "lxml": {"xpath_query", "xslt_transform"},
    "schematron": {"validate_part"},
    "ocr": {"extract_text"},
    "pypdf": {"extract_text"},
    "pdfminer": {"extract_text", "extract_layout"},
    "docxtpl": {"render_template"},
    "python_docx": {"style_audit"},
}


def provider_capabilities(provider: str) -> List[str]:
    return sorted(PROVIDER_REGISTRY.get(provider, set()))


def provider_names() -> List[str]:
    return sorted(PROVIDER_REGISTRY.keys())
