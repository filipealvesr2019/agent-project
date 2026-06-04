import os

folder = 'Assets/Icons'
for file in os.listdir(folder):
    if file.endswith('.svg'):
        filepath = os.path.join(folder, file)
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        content = content.replace('stroke="currentColor"', 'stroke="#000000"')
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
print('SVGs patched for JUCE compatibility!')