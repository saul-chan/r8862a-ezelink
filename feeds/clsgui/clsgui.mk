#
# Copyright (C) 2008-2015 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#
#$(error "error: cls-turnkey.mk is compile")
CLSGUI_NAME?=$(notdir ${CURDIR})
CLSGUI_TYPE?=$(word 2,$(subst -, ,$(CLSGUI_NAME)))
CLSGUI_BASENAME?=$(patsubst clsgui-$(CLSGUI_TYPE)-%,%,$(CLSGUI_NAME))
CLSGUI_LANGUAGES:=$(sort $(filter-out templates,$(notdir $(wildcard ${CURDIR}/po/*))))
CLSGUI_DEFAULTS:=$(notdir $(wildcard ${CURDIR}/root/etc/uci-defaults/*))
CLSGUI_PKGARCH?=$(if $(realpath src/Makefile),,all)
CLSGUI_SECTION?=clsgui
CLSGUI_CATEGORY?=ClourneySemi

# Language code titles
CLSGUI_LANG.en=English
CLSGUI_LANG.zh_Hans=简体中文 (Chinese Simplified)

# Submenu titles
CLSGUI_MENU=WebGUI

# Language aliases
CLSGUI_LC_ALIAS.zh_Hans=zh-cn

# Default locations
HTDOCS = /clsgui
LUA_LIBRARYDIR = /usr/lib/lua
CLSGUI_LIBRARYDIR = $(LUA_LIBRARYDIR)/luci

# 1: everything expect po subdir or only po subdir
define findrev
  $(shell \
    if git log -1 >/dev/null 2>/dev/null; then \
      set -- $$(git log -1 --format="%ct %h" --abbrev=7 -- $(if $(1),. ':(exclude)po',po)); \
      if [ -n "$$1" ]; then
        secs="$$(($$1 % 86400))"; \
        yday="$$(date --utc --date="@$$1" "+%y.%j")"; \
        printf '%s.%05d~%s' "$$yday" "$$secs" "$$2"; \
      else \
        echo "0"; \
      fi; \
    else \
      ts=$$(find . -type f $(if $(1),-not) -path './po/*' -printf '%T@\n' 2>/dev/null | sort -rn | head -n1 | cut -d. -f1); \
      if [ -n "$$ts" ]; then \
        secs="$$(($$ts % 86400))"; \
        date="$$(date --utc --date="@$$ts" "+%y%m%d")"; \
        printf '0.%s.%05d' "$$date" "$$secs"; \
      else \
        echo "0"; \
      fi; \
    fi \
  )
endef

PKG_NAME?=$(CLSGUI_NAME)
PKG_RELEASE?=1
PKG_INSTALL:=$(if $(realpath src/Makefile),1)
PKG_BUILD_DEPENDS += clsgui-base/host $(LUCI_BUILD_DEPENDS)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

PKG_PO_VERSION?=$(if $(DUMP),x,$(strip $(call findrev)))
PKG_SRC_VERSION?=$(if $(DUMP),x,$(strip $(call findrev,1)))

include $(INCLUDE_DIR)/package.mk

# CLSGUI_SUBMENU: the submenu-item below the LuCI top-level menu inside OpoenWrt menuconfig
#               usually one of the CLSGUI_MENU.* definitions
# CLSGUI_SUBMENU_DEFAULT: the regular SUBMENU defined by CLSGUI_TYPE or derrived from the packagename
# CLSGUI_SUBMENU_FORCED: manually forced value SUBMENU to set to by explicit definiton
#                      can be any string, "none" disables the creation of a submenu 
#                      most usefull in combination with CLSGUI_CATEGORY, to make the package appear
#                      anywhere in the menu structure
CLSGUI_SUBMENU_DEFAULT=$(if $(CLSGUI_MENU.$(CLSGUI_TYPE)),$(CLSGUI_MENU.$(CLSGUI_TYPE)),$(CLSGUI_MENU.app))
CLSGUI_SUBMENU=$(if $(CLSGUI_SUBMENU_FORCED),$(CLSGUI_SUBMENU_FORCED),$(CLSGUI_SUBMENU_DEFAULT))

define Package/$(PKG_NAME)
  SECTION:=$(CLSGUI_SECTION)
  CATEGORY:=$(CLSGUI_CATEGORY)
  SUBMENU:=$(CLSGUI_MENU)
  TITLE:=$(if $(CLSGUI_TITLE),$(CLSGUI_TITLE),LuCI $(CLSGUI_NAME) $(CLSGUI_TYPE))
  DEPENDS:=$(CLSGUI_DEPENDS)
  VERSION:=$(if $(PKG_VERSION),$(PKG_VERSION),$(PKG_SRC_VERSION))
  $(if $(CLSGUI_EXTRA_DEPENDS),EXTRA_DEPENDS:=$(CLSGUI_EXTRA_DEPENDS))
  $(if $(CLSGUI_PKGARCH),PKGARCH:=$(CLSGUI_PKGARCH))
  $(if $(PKG_PROVIDES),PROVIDES:=$(PKG_PROVIDES))
  URL:=$(CLSGUI_URL)
  MAINTAINER:=$(CLSGUI_MAINTAINER)
endef
                                
define Package/$(SUBMENU)/$(CLSGUI_SUBMENU)
  $(call Package/$(PKG_NAME))
  #TITLE:=$(if $(CLSGUI_TITLE),$(CLSGUI_TITLE),LuCI $(CLSGUI_NAME) $(CLSGUI_TYPE))
  #DEPENDS:=$(CLSGUI_DEPENDS)
  #VERSION:=$(if $(PKG_VERSION),$(PKG_VERSION),$(PKG_SRC_VERSION))
  #$(if $(CLSGUI_EXTRA_DEPENDS),EXTRA_DEPENDS:=$(CLSGUI_EXTRA_DEPENDS))
  #$(if $(CLSGUI_PKGARCH),PKGARCH:=$(CLSGUI_PKGARCH))
  #$(if $(PKG_PROVIDES),PROVIDES:=$(PKG_PROVIDES))
  #URL:=$(CLSGUI_URL)
  #MAINTAINER:=$(CLSGUI_MAINTAINER)
endef


ifneq ($(CLSGUI_DESCRIPTION),)
 define Package/$(PKG_NAME)/description
   $(strip $(CLSGUI_DESCRIPTION))
 endef
endif

define Build/Prepare
	for d in luasrc htdocs root src; do \
	  if [ -d ./$$$$d ]; then \
	    mkdir -p $(PKG_BUILD_DIR)/$$$$d; \
		$(CP) ./$$$$d/* $(PKG_BUILD_DIR)/$$$$d/; \
	  fi; \
	done
	$(call Build/Prepare/Default)
endef

define Build/Configure
endef

ifneq ($(wildcard ${CURDIR}/src/Makefile),)
 MAKE_PATH := src/
 MAKE_VARS += FPIC="$(FPIC)" CLSGUI_VERSION="$(PKG_SRC_VERSION)" CLSGUI_GITBRANCH="$(PKG_GITBRANCH)"

 define Build/Compile
	$(call Build/Compile/Default,clean compile)
 endef
else
 define Build/Compile
 endef
endif

CLSGUIBASE = clsgui-base

define Package/$(PKG_NAME)/install
	if [ -d $(PKG_BUILD_DIR)/luasrc ]; then \
	  $(INSTALL_DIR) $(1)$(CLSGUI_LIBRARYDIR); \
	  cp -pR $(PKG_BUILD_DIR)/luasrc/* $(1)$(CLSGUI_LIBRARYDIR)/; \
	  $(FIND) $(1)$(CLSGUI_LIBRARYDIR)/ -type f -name '*.luadoc' | $(XARGS) rm; \
	  $(if $(CONFIG_CLSGUI_SRCDIET),$(call SrcDiet,$(1)$(CLSGUI_LIBRARYDIR)/),true); \
	  $(call SubstituteVersion,$(1)$(CLSGUI_LIBRARYDIR)/) \
	else true; fi
	if [ -d $(PKG_BUILD_DIR)/htdocs ]; then \
	  $(INSTALL_DIR) $(1)$(HTDOCS); \
	  cp -pR $(PKG_BUILD_DIR)/htdocs/* $(1)$(HTDOCS)/; \
	  if [ "$(PKG_NAME)" = "$(CLSGUIBASE)" ]; then \
		echo "This is mytest"; \
		$(LN) /www/cgi-bin $(1)$(HTDOCS)/cgi-bin; \
		$(LN) /www/index.html $(1)$(HTDOCS)/index.html; \
		$(LN) /www/luci-static/resources/cbi $(1)$(HTDOCS)/luci-static/resources/cbi; \
		$(LN) /www/luci-static/resources/cbi.js $(1)$(HTDOCS)/luci-static/resources/cbi.js; \
		$(LN) /www/luci-static/resources/firewall.js $(1)$(HTDOCS)/luci-static/resources/firewall.js; \
		$(LN) /www/luci-static/resources/form.js $(1)$(HTDOCS)/luci-static/resources/form.js; \
		$(LN) /www/luci-static/resources/fs.js $(1)$(HTDOCS)/luci-static/resources/fs.js; \
		$(LN) /www/luci-static/resources/icons $(1)$(HTDOCS)/luci-static/resources/icons; \
		$(LN) /www/luci-static/resources/luci.js $(1)$(HTDOCS)/luci-static/resources/luci.js; \
		$(LN) /www/luci-static/resources/network.js $(1)$(HTDOCS)/luci-static/resources/network.js; \
		$(LN) /www/luci-static/resources/plugin $(1)$(HTDOCS)/luci-static/resources/plugin; \
		$(LN) /www/luci-static/resources/promis.min.js $(1)$(HTDOCS)/luci-static/resources/promis.min.js; \
		$(LN) /www/luci-static/resources/protocol $(1)$(HTDOCS)/luci-static/resources/protocol; \
		$(LN) /www/luci-static/resources/rpc.js $(1)$(HTDOCS)/luci-static/resources/rpc.js; \
		$(LN) /www/luci-static/resources/tools/prng.js $(1)$(HTDOCS)/luci-static/resources/tools/prng.js; \
		$(LN) /www/luci-static/resources/tools/widgets.js $(1)$(HTDOCS)/luci-static/resources/tools/widgets.js; \
		$(LN) /www/luci-static/resources/uci.js $(1)$(HTDOCS)/luci-static/resources/uci.js; \
		$(LN) /www/luci-static/resources/ui.js $(1)$(HTDOCS)/luci-static/resources/ui.js; \
		$(LN) /www/luci-static/resources/validation.js $(1)$(HTDOCS)/luci-static/resources/validation.js; \
		$(LN) /www/luci-static/resources/xhr.js $(1)$(HTDOCS)/luci-static/resources/xhr.js; \
	  fi; \
	else true; fi
	if [ -d $(PKG_BUILD_DIR)/root ]; then \
	  $(INSTALL_DIR) $(1)/; \
	  cp -pR $(PKG_BUILD_DIR)/root/* $(1)/; \
	else true; fi
	if [ -d $(PKG_BUILD_DIR)/src ]; then \
	  $(call Build/Install/Default) \
	  $(CP) $(PKG_INSTALL_DIR)/* $(1)/; \
	else true; fi
endef

ifndef Package/$(PKG_NAME)/postinst
define Package/$(PKG_NAME)/postinst
[ -n "$${IPKG_INSTROOT}" ] || { \
	rm -f /tmp/luci-indexcache
	rm -rf /tmp/luci-modulecache/
	killall -HUP rpcd 2>/dev/null
	exit 0
}
endef
endif


# additional setting luci-cls-base package
ifeq ($(PKG_NAME),clsgui-base)
  define Package/clsgui-base/config
   menu "Translations"$(foreach lang,$(CLSGUI_LANGUAGES),$(if $(CLSGUI_LANG.$(lang)),

     config CLSGUI_LANG_$(lang)
	   tristate "$(shell echo '$(CLSGUI_LANG.$(lang))' | sed -e 's/^.* (\(.*\))$$/\1/') ($(lang))"))

   endmenu
 endef
endif


CLSGUI_BUILD_PACKAGES := $(PKG_NAME)

# 1: LuCI language code
# 2: BCP 47 language tag
define LuciTranslation
  define Package/luci-i18n-$(CLSGUI_BASENAME)-$(1)
    SECTION:=clsgui
    CATEGORY:=ClourneySemi
    TITLE:=$(PKG_NAME) - $(1) translation
    HIDDEN:=1
    DEFAULT:=CLSGUI_LANG_$(2)||(ALL&&m)
    DEPENDS:=$(PKG_NAME)
    VERSION:=$(PKG_PO_VERSION)
    PKGARCH:=all
  endef

  define Package/luci-i18n-$(CLSGUI_BASENAME)-$(1)/description
    Translation for $(PKG_NAME) - $(CLSGUI_LANG.$(2))
  endef

  define Package/luci-i18n-$(CLSGUI_BASENAME)-$(1)/install
	$$(INSTALL_DIR) $$(1)/etc/uci-defaults
	echo "uci set luci.languages.$(subst -,_,$(1))='$(CLSGUI_LANG.$(2))'; uci commit luci" \
		> $$(1)/etc/uci-defaults/luci-i18n-$(CLSGUI_BASENAME)-$(1)
	$$(INSTALL_DIR) $$(1)$(CLSGUI_LIBRARYDIR)/i18n
	$(foreach po,$(wildcard ${CURDIR}/po/$(2)/*.po), \
		po2lmo $(po) \
			$$(1)$(CLSGUI_LIBRARYDIR)/i18n/$(basename $(notdir $(po))).$(1).lmo;)
  endef

  define Package/luci-i18n-$(CLSGUI_BASENAME)-$(1)/postinst
	[ -n "$$$${IPKG_INSTROOT}" ] || {
		(. /etc/uci-defaults/luci-i18n-$(CLSGUI_BASENAME)-$(1)) && rm -f /etc/uci-defaults/luci-i18n-$(CLSGUI_BASENAME)-$(1)
		exit 0
	}
  endef

  CLSGUI_BUILD_PACKAGES += luci-i18n-$(CLSGUI_BASENAME)-$(1)

endef

$(foreach lang,$(CLSGUI_LANGUAGES),$(if $(CLSGUI_LANG.$(lang)),$(eval $(call LuciTranslation,$(firstword $(CLSGUI_LC_ALIAS.$(lang)) $(lang)),$(lang)))))
$(foreach pkg,$(CLSGUI_BUILD_PACKAGES),$(eval $(call BuildPackage,$(pkg))))
