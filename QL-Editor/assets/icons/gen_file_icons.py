import base64
from PIL import Image, ImageDraw, ImageFont

def create_file_icon(name, label, color, is_folder=False):
    img = Image.new('RGBA', (128, 128), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    if is_folder:
        # Folder shape
        draw.polygon([(16, 24), (48, 24), (64, 40), (112, 40), (112, 104), (16, 104)], fill=color)
        draw.polygon([(16, 48), (112, 48), (104, 104), (24, 104)], fill=(min(color[0]+30,255), min(color[1]+30,255), min(color[2]+30,255), 255))
    else:
        # Document shape with folded corner
        draw.polygon([(24, 16), (80, 16), (104, 40), (104, 112), (24, 112)], fill=color)
        draw.polygon([(80, 16), (80, 40), (104, 40)], fill=(min(color[0]+40,255), min(color[1]+40,255), min(color[2]+40,255), 255))
        
        # Add label text if provided
        if label:
            # We don't have a specific font, so we draw it manually or use default font
            try:
                font = ImageFont.truetype("arial.ttf", 24)
            except:
                font = ImageFont.load_default()
            
            # Simple text centering
            # Since load_default might be too small, we can scale it up
            txt_img = Image.new('RGBA', (128, 128), (0,0,0,0))
            txt_draw = ImageDraw.Draw(txt_img)
            txt_draw.text((32, 64), label, font=font, fill=(255,255,255,255))
            img = Image.alpha_composite(img, txt_img)
            
    img.save(name)

# Base colors
C_BLUE = (50, 150, 250, 255)
C_GREEN = (50, 200, 80, 255)
C_ORANGE = (250, 150, 50, 255)
C_PURPLE = (150, 50, 200, 255)
C_RED = (220, 50, 50, 255)
C_YELLOW = (230, 200, 100, 255)
C_GRAY = (150, 150, 150, 255)

create_file_icon('DirectoryIcon.png', '', C_YELLOW, True)
create_file_icon('FileIcon.png', 'FILE', C_GRAY, False)
create_file_icon('Icon_FBX.png', '3D', C_ORANGE, False)
create_file_icon('Icon_PNG.png', 'IMG', C_RED, False)
create_file_icon('Icon_WAV.png', 'SND', C_PURPLE, False)
create_file_icon('Icon_UASSET.png', 'AST', C_BLUE, False)
create_file_icon('Icon_UMAP.png', 'MAP', C_GREEN, False)
