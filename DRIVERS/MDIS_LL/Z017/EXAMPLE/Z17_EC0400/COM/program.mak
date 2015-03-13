#***************************  M a k e f i l e  *******************************
#
#         Author: kp
#          $Date: 2005/04/27 14:10:15 $
#      $Revision: 1.1 $
#
#    Description: Makefile definitions for the Z17 example program
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#   Revision 1.1  2005/04/27 14:10:15  kp
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2005 by MEN mikro elektronik GmbH, Nuernberg, Germany
#*****************************************************************************

MAK_NAME=z17_ec0400

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)	\
			$(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)	\

MAK_INCL=$(MEN_INC_DIR)/z17_drv.h	\
         $(MEN_INC_DIR)/men_typs.h	\
         $(MEN_INC_DIR)/mdis_api.h	\
         $(MEN_INC_DIR)/usr_oss.h	\

MAK_INP1=z17_ec0400$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
