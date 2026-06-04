import sys

with open('src/UI/SidebarComponent.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

content = content.replace('BinaryData::building_2_svg', 'BinaryData::building2_svg')
content = content.replace('BinaryData::folder_kanban_svg', 'BinaryData::folderkanban_svg')
content = content.replace('BinaryData::message_square_svg', 'BinaryData::messagesquare_svg')

with open('src/UI/SidebarComponent.cpp', 'w', encoding='utf-8') as f:
    f.write(content)
print('Fixed identifier names!')