#!/usr/bin/env python3
from __future__ import annotations

import argparse
import base64
import importlib
import io
import os
import traceback
from typing import Dict, List, Tuple


ALLOWED_PROVIDERS = {
    "system": {"probe"},
    "smoke": {"ping"},
    "mammoth": {"docx_to_html"},
    "docxcompose": {"compose_append"},
    "lxml": {"xpath_query", "xslt_transform"},
}


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
    with open(path, "w", encoding="utf-8") as f:
        for k, v in values.items():
            v = v.replace("\\", "\\\\").replace("\n", "\\n")
            f.write(f"{k}={v}\n")


def unescape(value: str) -> str:
    return value.replace("\\n", "\n").replace("\\\\", "\\")


def decode_payload(values: Dict[str, str]) -> str:
    encoded = values.get("payload_b64", "")
    if not encoded:
        return ""
    return base64.b64decode(encoded.encode("utf-8")).decode("utf-8", errors="replace")


def encode_text(text: str) -> str:
    return base64.b64encode(text.encode("utf-8")).decode("utf-8")


def module_version(module_name: str) -> Tuple[bool, str, str]:
    try:
        mod = importlib.import_module(module_name)
        version = getattr(mod, "__version__", "unknown")
        return True, str(version), ""
    except Exception as ex:
        return False, "", str(ex)


def pack_providers(providers: List[Tuple[str, bool, str, str]]) -> str:
    chunks = []
    for name, avail, version, message in providers:
        n = name.replace("|", "/")
        v = str(version).replace("|", "/")
        m = str(message).replace("|", "/").replace(";", ",")
        chunks.append(f"{n}|{'1' if avail else '0'}|{v}|{m}")
    return ";".join(chunks)


def provider_probe() -> Dict[str, str]:
    providers = [
        ("smoke", True, "builtin", "available"),
    ]

    ok, ver, msg = module_version("mammoth")
    providers.append(("mammoth", ok, ver, "available" if ok else msg))

    okc, verc, msgc = module_version("docxcompose")
    providers.append(("docxcompose", okc, verc, "available" if okc else msgc))

    okl, verl, msgl = module_version("lxml")
    providers.append(("lxml", okl, verl, "available" if okl else msgl))

    return {
        "code": "ok",
        "message": "provider probe complete",
        "providers": pack_providers(providers),
    }


def provider_smoke_ping() -> Dict[str, str]:
    return {
        "code": "ok",
        "message": "smoke provider alive",
        "text_b64": encode_text("pong"),
    }


def provider_mammoth_docx_to_html(input_path: str) -> Dict[str, str]:
    try:
        import mammoth  # type: ignore
    except Exception as ex:
        return {"code": "provider_unavailable", "message": f"mammoth unavailable: {ex}"}

    if not input_path or not os.path.exists(input_path):
        return {"code": "invalid_request", "message": "input_path for mammoth is required"}

    with open(input_path, "rb") as docx_file:
        result = mammoth.convert_to_html(docx_file)
    html = result.value
    return {
        "code": "ok",
        "message": "mammoth conversion complete",
        "text_b64": encode_text(html),
    }


def provider_docxcompose_append(base_path: str, append_path: str, output_path: str) -> Dict[str, str]:
    try:
        from docx import Document  # type: ignore
        from docxcompose.composer import Composer  # type: ignore
    except Exception as ex:
        return {"code": "provider_unavailable", "message": f"docxcompose unavailable: {ex}"}

    if not base_path or not append_path or not output_path:
        return {
            "code": "invalid_request",
            "message": "base input_path, append payload, and output_path are required",
        }

    if not os.path.exists(base_path):
        return {"code": "invalid_request", "message": "base DOCX path not found"}
    if not os.path.exists(append_path):
        return {"code": "invalid_request", "message": "append DOCX path not found"}

    base = Document(base_path)
    composer = Composer(base)
    composer.append(Document(append_path))
    composer.save(output_path)

    return {
        "code": "ok",
        "message": "docxcompose append complete",
        "output_path": output_path,
    }


def provider_lxml_xpath_query(input_path: str, xpath_expr: str) -> Dict[str, str]:
    try:
        from lxml import etree  # type: ignore
    except Exception as ex:
        return {"code": "provider_unavailable", "message": f"lxml unavailable: {ex}"}

    if not input_path or not xpath_expr or not os.path.exists(input_path):
        return {"code": "invalid_request", "message": "input_path and xpath expression are required"}

    tree = etree.parse(input_path)
    result = tree.xpath(xpath_expr)
    text = "\n".join([str(item) for item in result])
    return {
        "code": "ok",
        "message": "xpath query complete",
        "text_b64": encode_text(text),
    }


def provider_lxml_xslt_transform(input_path: str, xslt_text: str) -> Dict[str, str]:
    try:
        from lxml import etree  # type: ignore
    except Exception as ex:
        return {"code": "provider_unavailable", "message": f"lxml unavailable: {ex}"}

    if not input_path or not xslt_text or not os.path.exists(input_path):
        return {"code": "invalid_request", "message": "input_path and xslt payload are required"}

    xml_doc = etree.parse(input_path)
    xslt_doc = etree.parse(io.BytesIO(xslt_text.encode("utf-8")))
    transform = etree.XSLT(xslt_doc)
    output = transform(xml_doc)
    return {
        "code": "ok",
        "message": "xslt transform complete",
        "text_b64": encode_text(str(output)),
    }


def run(values: Dict[str, str]) -> Dict[str, str]:
    provider = unescape(values.get("provider", ""))
    operation = unescape(values.get("operation", ""))
    input_path = unescape(values.get("input_path", ""))
    output_path = unescape(values.get("output_path", ""))
    payload = decode_payload(values)

    if provider not in ALLOWED_PROVIDERS:
        return {"code": "invalid_request", "message": f"provider not allowed: {provider}"}
    if operation not in ALLOWED_PROVIDERS[provider]:
        return {"code": "invalid_request", "message": f"operation not allowed: {provider}.{operation}"}

    if provider == "system" and operation == "probe":
        return provider_probe()
    if provider == "smoke" and operation == "ping":
        return provider_smoke_ping()
    if provider == "mammoth" and operation == "docx_to_html":
        return provider_mammoth_docx_to_html(input_path)
    if provider == "docxcompose" and operation == "compose_append":
        return provider_docxcompose_append(input_path, payload, output_path)
    if provider == "lxml" and operation == "xpath_query":
        return provider_lxml_xpath_query(input_path, payload)
    if provider == "lxml" and operation == "xslt_transform":
        return provider_lxml_xslt_transform(input_path, payload)

    return {"code": "invalid_request", "message": "unhandled provider operation"}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--request", required=True)
    parser.add_argument("--response", required=True)
    args = parser.parse_args()

    try:
        request_values = read_kv(args.request)
        response_values = run(request_values)
    except Exception as ex:
        response_values = {
            "code": "operation_failed",
            "message": f"worker exception: {ex}",
            "text_b64": encode_text(traceback.format_exc()),
        }

    write_kv(args.response, response_values)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
