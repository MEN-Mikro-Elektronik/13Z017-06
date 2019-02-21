#***************************  M a k e f i l e  *******************************
#
#         Author: michael.roth@men.de
#          $Date: 2010/04/09 10:36:50 $
#      $Revision: 1.1 $
#
#    Description: Makefile definitions for the Z127_IO tool
#
#-----------------------------------------------------------------------------
#   Copyright (c) 2010-2019, MEN Mikro Elektronik GmbH
#*****************************************************************************

MAK_NAME=z127_io

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)	\
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)	\
         $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_utl$(LIB_SUFFIX)

MAK_INCL=$(MEN_INC_DIR)/men_typs.h	\
         $(MEN_INC_DIR)/usr_utl.h	\
         $(MEN_INC_DIR)/mdis_api.h	\
         $(MEN_INC_DIR)/usr_oss.h	\
         $(MEN_INC_DIR)/z17_drv.h

MAK_INP1=z127_io$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
