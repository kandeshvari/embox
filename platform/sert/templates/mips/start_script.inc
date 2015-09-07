"mkdir -m 777 /mandatory_test",
"mkramdisk -s 0x20000 /dev/ram1",
"dd if=/mndt.img of=/dev/ram1",
"mount -t ext2 /dev/ram1 /mandatory_test",

"smac_adm -R high_label -o low_label  -a r",
"smac_adm -R low_label  -o high_label -a w",
"smac_adm -R _          -o smac_admin -a r",

"smac_adm -R confidentially -o unclassified  -a r",
"smac_adm -R confidentially -o service  -a r",
"smac_adm -R confidentially -o secret  -a r",

"smac_adm -R secret -o unclassified  -a r",
"smac_adm -R secret -o service  -a r",

"smac_adm -R service -o unclassified  -a r",
"login",