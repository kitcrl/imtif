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

COPT += -D__DEBUG__=1


PROJ_PATH = .
COPT += -D__TRIGONOMETRIC__=1
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
	COPT += -D__CUSTOM_X_SOCKET__=0
	COPT += -D__CUSTOM_X_HTTP__=0
	COPT += -D__CUSTOM_X_HTTPD__=0
	COPT += -D__CUSTOM_X_HTTPSD__=0
	COPT += -D__CUSTOM_X_WS__=0
	COPT += -D__CUSTOM_X_SERIAL__=1
	COPT += -D__CUSTOM_X_SIFR__=0
	COPT += -D__SSL_TLS__=0
	COPT += -D__MTIF_LIMITER__=0
endif

ifeq ($(ONE_SHOT), yes)
	COPT += -D__STATIC_LIB__=1
	xLIB += $(PROJ_PATH)/../mtif/$(XTARGET)/libmtif.x.$(XTARGET).a
else
	COPT += -D__STATIC_LIB__=0
endif




SRC  = .
SRC += $(PROJ_PATH)/.
SRC += $(PROJ_PATH)/i

INC  = -I.
INC += -I$(PROJ_PATH)/.
INC += -I$(PROJ_PATH)/i

ifeq ($(CUSTOM), yes)
  _TARGET_NAME = $(OUTPUT_NAME).xf
endif
ifeq ($(CUSTOM), no)
  _TARGET_NAME = $(OUTPUT_NAME)
endif

ifeq ($(ONE_SHOT), yes)
  _TARGET_NAME = $(OUTPUT_NAME)
endif


INC += -I$(PROJ_PATH)/../mtif/$(XTARGET)
ifeq ($(CUSTOM), no)
INC += -I$(PROJ_PATH)/../mtif/openssl/v1.1.1k/$(XTARGET)/include
INC += -I$(PROJ_PATH)/../mtif/openssl/v1.1.1k/$(XTARGET)/include/openssl
xLIB += $(PROJ_PATH)/../mtif/openssl/v1.1.1k/$(XTARGET)/lib/libcrypto.a
xLIB += $(PROJ_PATH)/../mtif/openssl/v1.1.1k/$(XTARGET)/lib/libssl.a
endif
