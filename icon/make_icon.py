import os

src = 'capturer'

res = {
    "16x16",
    "20x20",
    "24x24",
    "30x30",
    "32x32",
    "36x36",
    "40x40",
    "48x48",
    "60x60",
    "64x64",
    "72x72",
    "80x80",
    "96x96",
    "128x128",
    "256x256",
    "320x320",
    "384x384",
    "512x512"
}

png_src = ''

for r in res:
    os.system('magick convert -background none -size {r} {src}.svg {src}_{r}.png'.format(src=src, r=r))
    png_src += '{src}_{r}.png '.format(src=src, r=r)

os.system('magick convert {png_src} {src}.ico'.format(src=src, png_src=png_src))
