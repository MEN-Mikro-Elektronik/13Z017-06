#***************************  M a k e f i l e  *******************************
#
#         Author: dieter.pfeuffer@men.de
#          $Date: 2017/05/03 16:53:38 $
#      $Revision: 1.1 $
#
#    Description: Makefile definitions for the Z17_Z127-01 driver variant
#
#-----------------------------------------------------------------------------
#   Copyright (c) 2016-2019, MEN Mikro Elektronik GmbH
#*****************************************************************************

MAK_NAME=z17_z127v01

MAK_SWITCH=$(SW_PREFIX)MAC_MEM_MAPPED \
            $(SW_PREFIX)Z17_VARIANT=Z17_Z127V01 \
			$(SW_PREFIX)Z17_MODEL_Z127 \
			$(SW_PREFIX)Z127V01

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/desc$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/oss$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/dbg$(LIB_SUFFIX)


MAK_INCL=$(MEN_INC_DIR)/z17_drv.h	\
         $(MEN_INC_DIR)/arwen.h	\
         $(MEN_INC_DIR)/men_typs.h	\
         $(MEN_INC_DIR)/oss.h		\
         $(MEN_INC_DIR)/mdis_err.h	\
         $(MEN_INC_DIR)/maccess.h	\
         $(MEN_INC_DIR)/desc.h		\
         $(MEN_INC_DIR)/mdis_api.h	\
         $(MEN_INC_DIR)/mdis_com.h	\
         $(MEN_INC_DIR)/modcom.h	\
         $(MEN_INC_DIR)/ll_defs.h	\
         $(MEN_INC_DIR)/ll_entry.h	\
         $(MEN_INC_DIR)/dbg.h

MAK_INP1=z17_drv$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
