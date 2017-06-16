#
# Makefile for the RDA framebuffer driver
#
subdir-ccflags-y := -Werror
#TODO - remove this!!

obj-$(CONFIG_RDA_GOUDA)        += rda_gouda.o

obj-$(CONFIG_FB_RDA)           += rdafb.o
rdafb-y                        := rda_fb.o
rdafb-y                        += rda_panel.o
rdafb-objs                     := $(rdafb-y)
