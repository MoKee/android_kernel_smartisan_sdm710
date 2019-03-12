#
# Makefile for the qti specific video device drivers
# based on V4L2.
#
obj-$(CONFIG_MSM_VIDC_V4L2) += vidc/
obj-$(CONFIG_MSM_VIDC_3X_V4L2) += vidc_3x/
obj-y += sde/
ifdef CONFIG_VENDOR_SMARTISAN
obj-$(CONFIG_SPECTRA_CAMERA) += camera_smartisan/
else
obj-$(CONFIG_SPECTRA_CAMERA) += camera/
endif
obj-y += broadcast/
obj-$(CONFIG_DVB_MPQ) += dvb/
