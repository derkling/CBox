# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.4

@AXIS_CONFIG@

SUBDIRS = src

MMC_LOCAL = ./mmc
MMC_NFS = /home/derkling/Personal/Work/Daricom/LocalSVN/target/mnt/flash/mmc
MMC_BINs = src/cbox src/cboxtest

update-mmc:
	cp -v ${MMC_BINs} ${MMC_LOCAL}/bin
	cp -rf ${MMC_LOCAL} ${MMC_NFS}
