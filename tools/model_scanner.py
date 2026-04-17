import os
import re
import json

def scan_models(project_dir):
    missing_models = []
    pattern = re.compile(r'["\']([^"\']+\.(?:obj|fbx|gltf|uasset|umap))["\']', re.IGNORECASE)
    
    for root, dirs, files in os.walk(project_dir):
        if 'vendor' in root or '.git' in root or 'bin' in root or 'obj' in root:
            continue
        for file in files:
            if file.endswith(('.cpp', '.h', '.json', '.ini', '.py', '.prefab', '.scene')):
                file_path = os.path.join(root, file)
                try:
                    with open(file_path, 'r', encoding='utf-8') as f:
                        content = f.read()
                        matches = pattern.findall(content)
                        for match in matches:
                            # Assume path is relative to the project directory
                            full_path = os.path.join(project_dir, match)
                            if not os.path.exists(full_path):
                                missing_models.append({
                                    "file": file_path,
                                    "reference": match
                                })
                except Exception as e:
                    pass
    return missing_models

if __name__ == "__main__":
    report = []
    for proj in ["QL-Editor", "Sandbox"]:
        if os.path.exists(proj):
            missing = scan_models(proj)
            if missing:
                report.append(f"## {proj} 缺失模型引用:\n")
                for item in missing:
                    report.append(f"- 文件: `{item['file']}`\n  - 引用: `{item['reference']}`\n")
    
    report_content = "".join(report) if report else "未发现引用但缺失的模型。\n"
    print(report_content)
    with open("CleanupReport.md", "w", encoding="utf-8") as f:
        f.write("# 模型存在性扫描与清理报告\n\n")
        f.write(report_content)
