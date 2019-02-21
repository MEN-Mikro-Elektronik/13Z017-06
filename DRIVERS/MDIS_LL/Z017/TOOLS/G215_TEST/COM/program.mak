#***************************  M a k e f i l e  *******************************
#
#         Author: axel.morbach@men.de
#          $Date: 2011/03/28 11:09:01 $
#      $Revision: 1.1 $
#
#    Description: Makefile definitions for the G215 tool
#
#-----------------------------------------------------------------------------
#   Copyright (c) 2011-2019, MEN Mikro Elektronik GmbH
#*****************************************************************************

MAK_NAME=g215_test

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)	\
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_utl$(LIB_SUFFIX)	\

MAK_INCL=$(MEN_INC_DIR)/men_typs.h	\
         $(MEN_INC_DIR)/usr_utl.h	\
         $(MEN_INC_DIR)/mdis_api.h	\
         $(MEN_INC_DIR)/usr_oss.h	\
         $(MEN_INC_DIR)/z17_drv.h	\

MAK_INP1=g215_test$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
 