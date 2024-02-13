import ugfx, badge, consts

build                  = 0
name                   = "FIXME"
badge_name             = "FIXME"
dialog_title           = "Alert"
default_orientation    = 0

font_header            = "Roboto_Regular18"
font_default           = "Roboto_Regular12"

font_nickname_large    = "Roboto_Regular18"
font_nickname_small    = "Roboto_Regular12"
nick_width_large       = 24
nick_width_small       = 8
nick_height_large      = 36
nick_height_small      = 22

btn_ok = ugfx.BTN_START
btn_cancel = ugfx.BTN_B

hatcheryurl             = "https://"+consts.WOEZEL_WEB_SERVER
otacheckurl             = consts.OTA_WEB_PROTOCOL+"://"+consts.OTA_WEB_SERVER+":"+consts.OTA_WEB_PORT+consts.OTA_WEB_VERSION_PATH
