import xml.etree.ElementTree as ET
import os

items_path = '/home/user/Documentos/otbase/juan-canary/canary/data/items/items.xml'
output_dir = 'data/materials/tilesets'

# Map primarytype to output filename and Tileset Name
category_map = {
    'musical instruments': ('musical_instruments.xml', 'Musical Instruments'),
    'kitchen tools': ('kitchen.xml', 'Kitchen'),
    'remains': ('remains.xml', 'Remains'),
    'rocks': ('rocks.xml', 'Rocks'),
    'rubbish': ('rubbish.xml', 'Rubbish'),
    'refuse': ('refuse.xml', 'Refuse')
}

collected_items = {k: [] for k in category_map.keys()}

try:
    print("Parsing items.xml...")
    tree = ET.parse(items_path)
    root = tree.getroot()
    
    for item in root.findall('item'):
        item_id = int(item.get('id'))
        # Skip high IDs that might be OTB meta items if any (usually safest to include all from items.xml)
        
        p_type = None
        for attr in item.findall('attribute'):
            if attr.get('key') == 'primarytype':
                p_type = attr.get('value')
                break
        
        if p_type and p_type in collected_items:
            collected_items[p_type].append(item_id)

    # Generate XMLs
    for p_type, items in collected_items.items():
        filename, tileset_name = category_map[p_type]
        if not items:
            print(f"No items found for {p_type}")
            continue
            
        filepath = os.path.join(output_dir, filename)
        
        # Sort items
        items.sort()
        
        print(f"Writing {len(items)} items to {filename}...")
        
        with open(filepath, 'w') as f:
            f.write('<materials>\n')
            f.write(f'\t<tileset name="{tileset_name}">\n')
            f.write('\t\t<doodad>\n')
            
            for i in items:
                f.write(f'\t\t\t<item id="{i}"/>\n')
                
            f.write('\t\t</doodad>\n')
            f.write('\t</tileset>\n')
            f.write('</materials>\n')

    print("Done.")

except Exception as e:
    print(f"Error: {e}")
