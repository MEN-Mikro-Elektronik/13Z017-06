#***************************  M a k e f i l e  *******************************
#
#         Author: dieter.pfeuffer@men.de
#          $Date: 2017/05/03 16:53:53 $
#      $Revision: 1.1 $
#
#    Description: Makefile definitions for the Z127_OUT tool
#
#-----------------------------------------------------------------------------
#   Copyright (c) 2016-2019, MEN Mikro Elektronik GmbH
#*****************************************************************************

MAK_NAME=z127_out

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)	\
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_utl$(LIB_SUFFIX)

MAK_INCL=$(MEN_INC_DIR)/men_typs.h	\
         $(MEN_INC_DIR)/usr_utl.h	\
         $(MEN_INC_DIR)/mdis_api.h	\
         $(MEN_INC_DIR)/usr_oss.h	\
         $(MEN_INC_DIR)/z17_drv.h

MAK_INP1=z127_out$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
