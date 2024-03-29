################################################################################
#                                                                              #
#                                                                              #
#                                   PART I                                     #
#                                                                              #
#                                                                              #
################################################################################
################################################################################
#                                                                              #
#                                  COMMON                                      #
#                                                                              #
################################################################################
VERSION         = 2.0.0
LICENSE        ?= no

ARCH_TYPE      ?= x32
OUTPUT_TYPE ?= STATIC
#OUTPUT_TYPE ?= SHARED
#OUTPUT_TYPE ?= BINARY
################################################################################
#                                                                              #
#                                                                              #
#                                 PART II                                      #
#                                                                              #
#                                                                              #
################################################################################
################################################################################
#                                                                              #
#                                 COMPILER                                     #
#                                                                              #
################################################################################
COPT = -O2 -no-integrated-cpp
#COPT = -g -ggdb -W -Wall -no-integrated-cpp -static-libgcc
ifeq ($(DEBUG), yes)
	COPT += -D__DEBUG__=1
else
	COPT += -D__DEBUG__=0
endif

ifeq ($(ARCH_TYPE), x32)
	TOOLCHAIN_PATH = /usr
	XCOMPILE- = 
	XTARGET   = x32
	COPT += -Dx32
	COPT += -m32
	COPT += -DPACKED
	COPT += -D__LINUX__=1
	COPT += -D__PACKED__=1
	COPT += -D__uCHIP__=0
endif

ifeq ($(ARCH_TYPE), x64)
	TOOLCHAIN_PATH = /usr
	XCOMPILE- = 
	XTARGET   = x64
	COPT += -Dx64
	COPT += -m64
	COPT += -D__LINUX__=1
	COPT += -D__PACKED__=1
	COPT += -D__uCHIP__=0
endif

ifeq ($(ARCH_TYPE), a32) #####
	COPT += -Da32
	ifeq ($(ABI), HARD)
		TOOLCHAIN_PATH = /usr/local/ext/toolchain/x64/v7/arm-linux-gnueabihf/
		XCOMPILE- = arm-linux-gnueabihf-
		XTARGET = hf.a32
		COPT += -D__LINUX__=1
		COPT += -D__PACKED__=1
		COPT += -D__uCHIP__=0
		COPT += -DABI_HARD
		COPT += -mfloat-abi=hard
	endif
	ifeq ($(ABI), SOFT)
		TOOLCHAIN_PATH = /usr/local/ext/toolchain/x64/v7/arm-linux-gnueabi/
		XCOMPILE- = arm-linux-gnueabi-
		XTARGET = a32
		COPT += -D__LINUX__=1
		COPT += -D__PACKED__=1
		COPT += -D__uCHIP__=0
		COPT += -DABI_SOFT
		COPT += -mfloat-abi=soft
	endif
endif #####

ifeq ($(ARCH_TYPE), a64)
	COPT += -Da64
	TOOLCHAIN_PATH = /usr/local/ext/toolchain/x64/v7/aarch64-linux-gnu
	XCOMPILE- = aarch64-linux-gnu-
	ifeq ($(ABI), HARD)
		XTARGET   = hf.a64
		COPT += -D__LINUX__=1
		COPT += -D__PACKED__=1
		COPT += -D__uCHIP__=0
		COPT += -DABI_SOFT
		COPT += -march=armv8-a
	endif
	ifeq ($(ABI), SOFT)
		XTARGET   = a64
		COPT += -D__LINUX__=1
		COPT += -D__PACKED__=1
		COPT += -D__uCHIP__=0
		COPT += -DABI_HARD
		COPT += -march=armv8-a
	endif
endif

ifeq ($(ARCH_TYPE), m32) #####
	TOOLCHAIN_PATH = /usr/local/ext/toolchain/x64/v7/arm-eabi/
	XCOMPILE- = arm-eabi-
	XTARGET = m32
	COPT += -D__PACKED__=1
	COPT += -D__uCHIP__=1
endif #####


TOOLCHAIN_BIN  = $(TOOLCHAIN_PATH)/bin/
TOOLCHAIN_LIB = $(TOOLCHAIN_PATH)/lib
ifeq ($(ARCH_TYPE), x64)
TOOLCHAIN_LIB = $(TOOLCHAIN_PATH)/lib64
endif

CC     = $(TOOLCHAIN_BIN)$(XCOMPILE-)gcc
AR     = $(TOOLCHAIN_BIN)$(XCOMPILE-)ar
RANLIB = $(TOOLCHAIN_BIN)$(XCOMPILE-)ranlib
AR_OPT = rcs


################################################################################
#                                                                              #
#                                  SUFFIX                                      #
#                                                                              #
################################################################################
C_SUFFIX = .c
O_SUFFIX = .o

C_SUFFIX = .c
O_SUFFIX = .o

ifeq ($(OUTPUT_TYPE), SHARED)
	OUT_SUFFIX = .so
	COPT += -D__STATIC_LIB__=0
endif

ifeq ($(OUTPUT_TYPE), STATIC)
	OUT_SUFFIX = .a
	COPT += -D__STATIC_LIB__=1
endif

ifeq ($(OUTPUT_TYPE), BINARY)
	OUT_SUFFIX = .out
endif

################################################################################
#                                                                              #
#                              COMPILE OPTIONS                                 #
#                                                                              #
################################################################################

LIBS = -lnsl -lrt
RM = rm -rf
CP = cp -rf
MV = mv

################################################################################
#                                                                              #
#                                 DIRECTORIES                                  #
#                                                                              #
################################################################################
INC = .
SRC = .
LIB = lib
OBJ = obj
BIN = bin
OUT = out
COM = coms

################################################################################
################################################################################
#############################                 ##################################
#############################                 ##################################
#############################                 ##################################
#############################                 ##################################
##########################                       ###############################
###########################                     ################################
#############################                 ##################################
###############################             ####################################
#################################         ######################################
###################################     ########################################
##################################### ##########################################
################################################################################

################################################################################
#                                                                              #
#                                                                              #
#                            DIRECTORY INFORMATION                             #
#                                                                              #
#                                                                              #
################################################################################
include opt.mak

_NAME = $(_TARGET_NAME)
TARGET_OBJ = $(OUT)/$(_NAME).$(OBJ).$(XTARGET)
TARGET_NAME = $(_TARGET_NAME).$(XTARGET)

#                                                                              #
#                                                                              #
################################################################################
##################################### ##########################################
###################################     ########################################
#################################         ######################################
###############################             ####################################
#############################                 ##################################
###########################                     ################################
##########################                       ###############################
#############################                 ##################################
#############################                 ##################################
#############################                 ##################################
#############################                 ##################################
################################################################################
################################################################################

################################################################################
#                                                                              #
#                            SOURCE CONVERTING                                 #
#                                                                              #
################################################################################
C_SRCS  = $(foreach dir, $(SRC), $(wildcard $(dir)/*$(C_SUFFIX)))
CXX_SRCS = $(foreach dir, $(SRC), $(wildcard $(dir)/*$(CXX_SUFFIX)))
OBJS = $(foreach dir, . $(TARGET_OBJ), $(wildcard $(dir)/*$(O_SUFFIX)))
OXXS = $(foreach dir, . $(TARGET_OBJ), $(wildcard $(dir)/*$(OXX_SUFFIX)))

################################################################################
#                                                                              #
#                             COMPILE OBJECT                                   #
#                                                                              #
################################################################################
%$(O_SUFFIX) :
	@echo "################################################################################"
	@echo "#                                                                              #"
	@echo "#  compile "$@
	@echo "#                                                                              #"
	@echo "################################################################################"
	$(RM) $(TARGET_OBJ)/$@;
	$(CC) $(INC) $(COPT) -static -o $(addprefix $(TARGET_OBJ)/, $(notdir $@)) \
	-fPIC -c $(subst $(O_SUFFIX),$(C_SUFFIX), $@) -lm;
	@echo "################################################################################"
	@echo "#                                                                              #"
	@echo "#                                                                              #"
	@echo "#                                                                              #"
	@echo "################################################################################"

%$(OXX_SUFFIX) :
	@echo "################################################################################"
	@echo "#                                                                              #"
	@echo "#  compile CXX "$@
	@echo "#                                                                              #"
	@echo "################################################################################"
	$(CXX) $(INC) $(COPT) -static -o $(addprefix $(TARGET_OBJ)/, $(notdir $@)) \
	-fpermissive -fPIC -c $(subst $(OXX_SUFFIX),$(CXX_SUFFIX), $@) -lm;
	@echo "################################################################################"
	@echo "#                                                                              #"
	@echo "#                                                                              #"
	@echo "#                                                                              #"
	@echo "################################################################################"

################################################################################
#                                                                              #
#                                    BUILD                                     #
#                                                                              #
################################################################################
% :
ifeq ($(OUTPUT_TYPE), SHARED)
	@echo "################################################################################"
	@echo "#                                                                              #"
	@echo "#  shared object "$@$(OUT_SUFFIX)
	@echo "#                                                                              #"
	@echo "################################################################################"
	$(CC) -fPIC -shared $(COPT) -o $(OUT)/$@$(OUT_SUFFIX) $(OBJS) $(xLIB);
	@echo "################################################################################"
	@echo "#                                                                              #"
	@echo "#                                                                              #"
	@echo "#                                                                              #"
	@echo "################################################################################"
endif
ifeq ($(OUTPUT_TYPE), STATIC)
	@echo "################################################################################"
	@echo "#                                                                              #"
	@echo "#  archive "$@$(OUT_SUFFIX)
	@echo "#                                                                              #"
	@echo "################################################################################"
	$(AR) $(AR_OPT) $(OUT)/$@$(OUT_SUFFIX) $(OBJS) $(xLIB);
	$(RANLIB) $(OUT)/$@$(OUT_SUFFIX);
	@echo "################################################################################"
	@echo "#                                                                              #"
	@echo "#                                                                              #"
	@echo "#                                                                              #"
	@echo "################################################################################"
endif
ifeq ($(OUTPUT_TYPE), BINARY)
	@echo "################################################################################"
	@echo "#                                                                              #"
	@echo "#  binary "$@$(OUT_SUFFIX)
	@echo "#                                                                              #"
	@echo "################################################################################"
	$(CC) -o $(OUT)/$@$(OUT_SUFFIX) $(OBJS) $(INC) $(COPT) $(xLIB);
	@echo "################################################################################"
	@echo "#                                                                              #"
	@echo "#                                                                              #"
	@echo "#                                                                              #"
	@echo "################################################################################"
endif

all:
	@echo "################################################################################"
	@echo "#"
	@echo "#"
	@echo "#  Build Start "$(TARGET_NAME)"  ("$(ARCH_TYPE)")"
	@echo "#  "$(addsuffix $(O_SUFFIX),$(basename $(C_SRCS)))
	@echo "#"
	@echo "#"
	@echo "################################################################################"
	make $(addsuffix $(O_SUFFIX),$(basename $(C_SRCS)));
	@echo "################################################################################"
	@echo "#"
	@echo "#"
	@echo "#  link "$(TARGET_NAME)
	@echo "#"
	@echo "#"
	@echo "################################################################################"
	make $(TARGET_NAME);
	@echo "################################################################################"
	@echo "#"
	@echo "#"
	@echo "#  Build Complete "$(TARGET_NAME)"  ("$(ARCH_TYPE)")"
	@echo "#"
	@echo "#"
	@echo "################################################################################"
	@echo " "
	@echo " "
	@echo " "
	@echo " "
	@echo " "
	@echo " "
	@echo " "
	@echo " "
	@echo " "
	@echo " "

################################################################################
#                                                                              #
#                                 BUILD PREFIX                                 #
#                                                                              #
################################################################################
dir:
	mkdir -p $(OUT);
	mkdir -p $(TARGET_OBJ);

clean :
	$(RM)  *.o core;
	$(RM)  $(TARGET_OBJ)/*.o;
	$(RM)  $(OUT)/$(TARGET_NAME)$(OUT_SUFFIX);

distclean:
	$(RM)  *.o core;
	$(RM)  $(OUT)/*;
	$(RM)  $(TARGET_OBJ)/*.o;
	$(RM)  $(OUT)/*$(OUT_SUFFIX);
