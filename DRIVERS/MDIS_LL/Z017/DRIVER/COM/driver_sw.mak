#***************************  M a k e f i l e  *******************************
#
#         Author: ub
#          $Date: 2004/06/18 14:29:54 $
#      $Revision: 1.1 $
#
#    Description: Makefile definitions for the Z17 driver
#
#-----------------------------------------------------------------------------
#   Copyright (c) 2000-2019, MEN Mikro Elektronik GmbH
#*****************************************************************************

MAK_NAME=z17_sw

MAK_SWITCH=$(SW_PREFIX)MAC_MEM_MAPPED \
			$(SW_PREFIX)MAC_BYTESWAP  \
			$(SW_PREFIX)Z17_VARIANT=Z17_SW \

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/desc$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/oss$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/dbg$(LIB_SUFFIX)	\


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
         $(MEN_INC_DIR)/dbg.h		\

MAK_INP1=z17_drv$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
