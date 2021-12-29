################################################################################
#                                                                              #
#                                                                              #
#                             OPTIONAL INFORMATION                             #
#                                                                              #
################################################################################
OUTPUT_NAME = $(notdir $(shell pwd))
ifeq ($(ONE_SHOT), yes)
OUTPUT_NAME = libimtif
endif

COPT += -D__TRIGONOMETRIC__=1
COPT += -D__WEBSOCKET__=1
COPT += -D__HTTP__=1
COPT += -D__MIME__=1
COPT += -D__SHM__=1
COPT += -D__SERIAL__=1
COPT += -D__SOCKET__=1
COPT += -D__PACKET__=1
COPT += -D__SHA1__=1
COPT += -D__JSON__=1
COPT += -D__NIC__=0
COPT += -D__CGI__=0
COPT += -D__XML__=0
COPT += -D__DBC__=0
COPT += -D__LIST__=0
COPT += -D__QUEUE__=0
COPT += -D__STACK__=0
COPT += -D__SYSQUEUE__=0

COPT += -D__IO__=1

PROJ_PATH = .
ifeq ($(CUSTOM), no)
	COPT += -D__CUSTOM_X_SOCKET__=1
	COPT += -D__CUSTOM_X_HTTP__=1
	COPT += -D__CUSTOM_X_HTTPD__=1
	COPT += -D__CUSTOM_X_HTTPSD__=1
	COPT += -D__CUSTOM_X_WS__=1
	COPT += -D__CUSTOM_X_SERIAL__=1
	COPT += -D__CUSTOM_X_SIFR__=1
	COPT += -D__SSL_TLS__=1
	COPT += -D__MTIF_LIMITER__=0
endif
ifeq ($(CUSTOM), yes)
	COPT += -D__CUSTOM_X_SOCKET__=1
	COPT += -D__CUSTOM_X_HTTP__=1
	COPT += -D__CUSTOM_X_HTTPD__=1
	COPT += -D__CUSTOM_X_HTTPSD__=1
	COPT += -D__CUSTOM_X_WS__=1
	COPT += -D__CUSTOM_X_SERIAL__=1
	COPT += -D__CUSTOM_X_SIFR__=0
	COPT += -D__SSL_TLS__=r
	COPT += -D__MTIF_LIMITER__=0
endif

ifeq ($(ONE_SHOT), yes)
	xLIB += $(PROJ_PATH)/../mtif/$(XTARGET)/libmtif.x.$(XTARGET).a
	xLIB += $(PROJ_PATH)/../mtif/$(XTARGET)/libio.$(XTARGET).a
endif




SRC  = .
SRC += $(PROJ_PATH)/.
SRC += $(PROJ_PATH)/i

INC  = -I.
INC += -I$(PROJ_PATH)/.
INC += -I$(PROJ_PATH)/i

ifeq ($(CUSTOM), yes)
ifeq ($(ONE_SHOT), yes)
  _TARGET_NAME = $(OUTPUT_NAME)
endif
ifeq ($(ONE_SHOT), no)
  _TARGET_NAME = $(OUTPUT_NAME).xf
endif
endif

ifeq ($(CUSTOM), no)
  _TARGET_NAME = $(OUTPUT_NAME)
endif


INC += -I$(PROJ_PATH)/../mtif/$(XTARGET)
ifeq ($(CUSTOM), no)
INC += -I$(PROJ_PATH)/../mtif/openssl/v1.1.1k/$(XTARGET)/include
INC += -I$(PROJ_PATH)/../mtif/openssl/v1.1.1k/$(XTARGET)/include/openssl
xLIB += $(PROJ_PATH)/../mtif/openssl/v1.1.1k/$(XTARGET)/lib/libcrypto.a
xLIB += $(PROJ_PATH)/../mtif/openssl/v1.1.1k/$(XTARGET)/lib/libssl.a
endif
