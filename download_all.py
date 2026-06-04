import urllib.request
import os

icons = [
    'house.svg',
    'building-2.svg',
    'folder-kanban.svg',
    'users.svg',
    'message-square.svg',
    'settings.svg'
]

os.makedirs('Assets/Icons', exist_ok=True)
base_url = 'https://raw.githubusercontent.com/lucide-icons/lucide/main/icons/'

for icon in icons:
    print(f'Downloading {icon}...')
    try:
        req = urllib.request.urlopen(base_url + icon)
        data_str = req.read().decode('utf-8')
        
        # Patch stroke color for JUCE
        data_str = data_str.replace('stroke="currentColor"', 'stroke="#000000"')
        data = data_str.encode('utf-8')
        
        # Save to file
        with open(f'Assets/Icons/{icon}', 'wb') as f_icon:
            f_icon.write(data)
    except Exception as e:
        print(f'Failed {icon}: {e}')

print('Downloaded all SVGs to Assets/Icons/')