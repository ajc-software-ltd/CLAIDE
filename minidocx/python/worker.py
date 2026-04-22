#!/usr/bin/env python3
from __future__ import annotations

import argparse
import base64
import importlib
import io
import json
import os
import traceback
from typing import Dict, List, Tuple

PROTOCOL_VERSION = "1"

ALLOWED_PROVIDERS = {
    "system": {"probe"},
    "smoke": {"ping"},
    "mammoth": {"docx_to_html"},
    "mammoth_semantic": {"export_html", "extract_raw_text"},
    "docxcompose": {"compose_append"},
    "lxml": {"xpath_query", "xslt_transform"},
    "docxtpl": {"render_template"},
    "python_docx": {"style_audit"},
}

PROVIDER_CAPABILITIES = {
    "smoke": ["ping"],
    "mammoth": ["docx_to_html"],
    "mammoth_semantic": ["export_html", "extract_raw_text"],
    "docxcompose": ["compose_append"],
    "lxml": ["xpath_query", "xslt_transform"],
    "docxtpl": ["render_template"],
    "python_docx": ["style_audit"],
}


def escape(value: str) -> str:
    return value.replace("\\", "\\\\").replace("\n", "\\n")


def unescape(value: str) -> str:
    return value.replace("\\n", "\n").replace("\\\\", "\\")


def read_kv(path: str) -> Dict[str, str]:
    out: Dict[str, str] = {}
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\n")
            if "=" not in line:
                continue
            k, v = line.split("=", 1)
            out[k.strip()] = v
    return out


def write_kv(path: str, values: Dict[str, str]) -> None:
    values = {"protocol_version": PROTOCOL_VERSION, **values}
    with open(path, "w", encoding="utf-8") as f:
        for k, v in values.items():
            f.write(f"{k}={escape(str(v))}\n")


def decode_payload(values: Dict[str, str]) -> str:
    encoded = values.get("payload_b64", "")
    if not encoded:
        return ""
    try:
        return base64.b64decode(encoded.encode("utf-8")).decode("utf-8", errors="replace")
    except Exception:
        return ""


def encode_text(text: str) -> str:
    return base64.b64encode(text.encode("utf-8")).decode("utf-8")


def error(code: str, message: str, debug: str = "") -> Dict[str, str]:
    out = {"code": code, "message": message, "provenance": "python_provider"}
    if debug:
        out["debug_detail"] = debug
    return out


def module_version(module_name: str) -> Tuple[bool, str, str]:
    try:
        mod = importlib.import_module(module_name)
        version = getattr(mod, "__version__", "unknown")
        return True, str(version), ""
    except Exception as ex:
        return False, "", str(ex)


def pack_providers(providers: List[Tuple[str, bool, str, List[str], str]]) -> str:
    chunks = []
    for name, avail, version, capabilities, message in providers:
        caps = ",".join(capabilities)
        chunks.append(f"{escape(name)}|{'1' if avail else '0'}|{escape(version)}|{escape(caps)}|{escape(message)}")
    return ";".join(chunks)


def mammoth_messages_to_warning_text(messages: List[object]) -> str:
    lines = []
    for m in messages:
        text = getattr(m, "message", str(m))
        level = getattr(m, "type", "warning")
        lines.append(f"{level}:{text}")
    return "\n".join(lines)


def provider_probe() -> Dict[str, str]:
    providers: List[Tuple[str, bool, str, List[str], str]] = []

    providers.append(("smoke", True, "builtin", PROVIDER_CAPABILITIES["smoke"], "available"))

    ok, ver, msg = module_version("mammoth")
    providers.append(("mammoth", ok, ver, PROVIDER_CAPABILITIES["mammoth"], "available" if ok else msg))
    providers.append(("mammoth_semantic", ok, ver, PROVIDER_CAPABILITIES["mammoth_semantic"], "available" if ok else msg))

    okc, verc, msgc = module_version("docxcompose")
    providers.append(("docxcompose", okc, verc, PROVIDER_CAPABILITIES["docxcompose"], "available" if okc else msgc))

    okl, verl, msgl = module_version("lxml")
    providers.append(("lxml", okl, verl, PROVIDER_CAPABILITIES["lxml"], "available" if okl else msgl))

    okt, vert, msqt = module_version("docxtpl")
    providers.append(("docxtpl", okt, vert, PROVIDER_CAPABILITIES["docxtpl"], "available" if okt else msqt))

    okd, verd, msgd = module_version("docx")
    providers.append(("python_docx", okd, verd, PROVIDER_CAPABILITIES["python_docx"], "available" if okd else msgd))

    return {
        "code": "ok",
        "message": "provider probe complete",
        "provenance": "python_provider",
        "providers": pack_providers(providers),
    }


def provider_smoke_ping() -> Dict[str, str]:
    return {
        "code": "ok",
        "message": "smoke provider alive",
        "provenance": "python_provider",
        "text_b64": encode_text("pong"),
    }


def provider_mammoth_docx_to_html(input_path: str) -> Dict[str, str]:
    try:
        import mammoth  # type: ignore
    except Exception as ex:
        return error("provider_unavailable", f"mammoth unavailable: {ex}")

    if not input_path or not os.path.exists(input_path):
        return error("invalid_request", "input_path for mammoth is required")

    with open(input_path, "rb") as docx_file:
        result = mammoth.convert_to_html(docx_file)
    return {
        "code": "ok",
        "message": "mammoth conversion complete",
        "provider_version": getattr(mammoth, "__version__", "unknown"),
        "provenance": "python_provider",
        "text_b64": encode_text(result.value),
    }


def provider_mammoth_semantic_export_html(input_path: str, payload: str) -> Dict[str, str]:
    try:
        import mammoth  # type: ignore
    except Exception as ex:
        return error("provider_unavailable", f"mammoth unavailable: {ex}")

    if not input_path or not os.path.exists(input_path):
        return error("invalid_request", "input_path for mammoth_semantic.export_html is required")

    style_map = None
    include_default_style_map = True
    if payload:
        try:
            params = json.loads(payload)
            if not isinstance(params, dict):
                return error("invalid_request", "payload must be a JSON object")
            style_map = params.get("style_map")
            include_default_style_map = bool(params.get("include_default_style_map", True))
        except Exception as ex:
            return error("invalid_request", f"invalid payload JSON: {ex}")

    kwargs = {"include_default_style_map": include_default_style_map}
    if style_map:
        kwargs["style_map"] = style_map

    with open(input_path, "rb") as docx_file:
        result = mammoth.convert_to_html(docx_file, **kwargs)

    warnings = mammoth_messages_to_warning_text(result.messages)

    out = {
        "code": "ok",
        "message": "mammoth semantic html export complete",
        "provider_version": getattr(mammoth, "__version__", "unknown"),
        "provenance": "python_provider",
        "text_b64": encode_text(result.value),
    }
    if warnings:
        out["warnings_b64"] = encode_text(warnings)
    return out


def provider_mammoth_semantic_extract_text(input_path: str) -> Dict[str, str]:
    try:
        import mammoth  # type: ignore
    except Exception as ex:
        return error("provider_unavailable", f"mammoth unavailable: {ex}")

    if not input_path or not os.path.exists(input_path):
        return error("invalid_request", "input_path for mammoth_semantic.extract_raw_text is required")

    with open(input_path, "rb") as docx_file:
        result = mammoth.extract_raw_text(docx_file)

    warnings = mammoth_messages_to_warning_text(getattr(result, "messages", []))

    out = {
        "code": "ok",
        "message": "mammoth semantic raw text extraction complete",
        "provider_version": getattr(mammoth, "__version__", "unknown"),
        "provenance": "python_provider",
        "text_b64": encode_text(result.value),
    }
    if warnings:
        out["warnings_b64"] = encode_text(warnings)
    return out


def provider_docxcompose_append(base_path: str, append_path: str, output_path: str) -> Dict[str, str]:
    try:
        from docx import Document  # type: ignore
        from docxcompose.composer import Composer  # type: ignore
    except Exception as ex:
        return error("provider_unavailable", f"docxcompose unavailable: {ex}")

    if not base_path or not append_path or not output_path:
        return error("invalid_request", "base input_path, append payload, and output_path are required")

    if not os.path.exists(base_path):
        return error("invalid_request", "base DOCX path not found")
    if not os.path.exists(append_path):
        return error("invalid_request", "append DOCX path not found")

    base = Document(base_path)
    composer = Composer(base)
    composer.append(Document(append_path))
    composer.save(output_path)

    return {
        "code": "ok",
        "message": "docxcompose append complete",
        "provenance": "python_provider",
        "output_path": output_path,
    }


def provider_lxml_xpath_query(input_path: str, xpath_expr: str) -> Dict[str, str]:
    try:
        from lxml import etree  # type: ignore
    except Exception as ex:
        return error("provider_unavailable", f"lxml unavailable: {ex}")

    if not input_path or not xpath_expr or not os.path.exists(input_path):
        return error("invalid_request", "input_path and xpath expression are required")

    tree = etree.parse(input_path)
    result = tree.xpath(xpath_expr)
    return {
        "code": "ok",
        "message": "xpath query complete",
        "provenance": "python_provider",
        "text_b64": encode_text("\n".join([str(item) for item in result])),
    }


def provider_lxml_xslt_transform(input_path: str, xslt_text: str) -> Dict[str, str]:
    try:
        from lxml import etree  # type: ignore
    except Exception as ex:
        return error("provider_unavailable", f"lxml unavailable: {ex}")

    if not input_path or not xslt_text or not os.path.exists(input_path):
        return error("invalid_request", "input_path and xslt payload are required")

    xml_doc = etree.parse(input_path)
    xslt_doc = etree.parse(io.BytesIO(xslt_text.encode("utf-8")))
    output = etree.XSLT(xslt_doc)(xml_doc)

    return {
        "code": "ok",
        "message": "xslt transform complete",
        "provenance": "python_provider",
        "text_b64": encode_text(str(output)),
    }


def provider_docxtpl_render_template(template_path: str, context_json: str, output_path: str) -> Dict[str, str]:
    try:
        from docxtpl import DocxTemplate  # type: ignore
    except Exception as ex:
        return error("provider_unavailable", f"docxtpl unavailable: {ex}")

    if not template_path or not output_path or not os.path.exists(template_path):
        return error("invalid_request", "template input_path and output_path are required")

    try:
        context = json.loads(context_json) if context_json else {}
        if not isinstance(context, dict):
            return error("invalid_request", "template context payload must be a JSON object")
    except Exception as ex:
        return error("invalid_request", f"invalid template context JSON: {ex}")

    tpl = DocxTemplate(template_path)
    tpl.render(context)
    tpl.save(output_path)

    return {
        "code": "ok",
        "message": "docxtpl render complete",
        "provenance": "python_provider",
        "output_path": output_path,
    }


def provider_python_docx_style_audit(input_path: str) -> Dict[str, str]:
    try:
        from docx import Document  # type: ignore
    except Exception as ex:
        return error("provider_unavailable", f"python-docx unavailable: {ex}")

    if not input_path or not os.path.exists(input_path):
        return error("invalid_request", "input_path is required for style audit")

    doc = Document(input_path)
    styles = []
    style_names = set()
    for style in doc.styles:
        style_names.add(style.name)
        styles.append({
            "name": style.name,
            "type": str(style.type),
            "builtin": bool(getattr(style, "builtin", False)),
        })

    missing_references = []
    for idx, para in enumerate(doc.paragraphs):
        if para.style is None:
            continue
        if para.style.name not in style_names:
            missing_references.append({"kind": "paragraph", "index": idx, "style": para.style.name})

    report = {
        "style_count": len(styles),
        "styles": styles,
        "missing_references": missing_references,
    }

    return {
        "code": "ok",
        "message": "python-docx style audit complete",
        "provenance": "python_provider",
        "text_b64": encode_text(json.dumps(report, ensure_ascii=False)),
    }


def validate_request(values: Dict[str, str]) -> Tuple[bool, Dict[str, str]]:
    required = ["protocol_version", "provider", "operation"]
    for field in required:
        if field not in values:
            return False, error("invalid_request", f"missing required field: {field}")

    if values.get("protocol_version") != PROTOCOL_VERSION:
        return False, error("protocol_mismatch", "request protocol version mismatch")

    provider = unescape(values.get("provider", ""))
    operation = unescape(values.get("operation", ""))

    if provider not in ALLOWED_PROVIDERS:
        return False, error("invalid_request", f"provider not allowed: {provider}")
    if operation not in ALLOWED_PROVIDERS[provider]:
        return False, error("invalid_request", f"operation not allowed: {provider}.{operation}")

    return True, {}


def run(values: Dict[str, str]) -> Dict[str, str]:
    ok, err = validate_request(values)
    if not ok:
        return err

    provider = unescape(values.get("provider", ""))
    operation = unescape(values.get("operation", ""))
    input_path = unescape(values.get("input_path", ""))
    output_path = unescape(values.get("output_path", ""))
    payload = decode_payload(values)

    if provider == "system" and operation == "probe":
        return provider_probe()
    if provider == "smoke" and operation == "ping":
        return provider_smoke_ping()
    if provider == "mammoth" and operation == "docx_to_html":
        return provider_mammoth_docx_to_html(input_path)
    if provider == "mammoth_semantic" and operation == "export_html":
        return provider_mammoth_semantic_export_html(input_path, payload)
    if provider == "mammoth_semantic" and operation == "extract_raw_text":
        return provider_mammoth_semantic_extract_text(input_path)
    if provider == "docxcompose" and operation == "compose_append":
        return provider_docxcompose_append(input_path, payload, output_path)
    if provider == "lxml" and operation == "xpath_query":
        return provider_lxml_xpath_query(input_path, payload)
    if provider == "lxml" and operation == "xslt_transform":
        return provider_lxml_xslt_transform(input_path, payload)
    if provider == "docxtpl" and operation == "render_template":
        return provider_docxtpl_render_template(input_path, payload, output_path)
    if provider == "python_docx" and operation == "style_audit":
        return provider_python_docx_style_audit(input_path)

    return error("invalid_request", "unhandled provider operation")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--request", required=True)
    parser.add_argument("--response", required=True)
    args = parser.parse_args()

    try:
        request_values = {k: unescape(v) for k, v in read_kv(args.request).items()}
        response_values = run(request_values)
    except Exception as ex:
        response_values = error("execution_failed", f"worker exception: {ex}", traceback.format_exc())

    write_kv(args.response, response_values)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
