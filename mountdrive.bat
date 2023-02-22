C:\"Program Files"\OSFMount\OSFMount -a -t file -f ..\Dyski\drive.hdd -s 40M -o rw -m F:

move /y BOOTX64.EFI "F:\EFI\Boot\"

timeout 3

C:\"Program Files"\OSFMount\OSFMount -D -m F:

pause

