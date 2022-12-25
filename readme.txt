# How to let the system run:

1. start snc brocker in one shell and let it block (should always the be first application up and running):
snc_brocker

2. start ui (compiled for host / -DON_HOST=1) with mplayer as backend
ui -p mplayer

3. start the key reader (need to be in forground, to catch the key hits)
keysender
(use keys: I=up,j=left,k=right,m=down,s=select,q=quit)

4. load at least one database (with the snc sender command):
sender "echo add /home/yorn/dwhelper/" ui_db
(database should be within the given directory)

this could be used with add and sub for as many database files given

