#***************************  M a k e f i l e  *******************************
#
#         Author: michael.roth@men.de
#          $Date: 2011/11/16 15:24:36 $
#      $Revision: 1.2 $
#
#    Description: Makefile definitions for the Z17_Z127 driver variant
#
#-----------------------------------------------------------------------------
#   Copyright (c) 2009-2019, MEN Mikro Elektronik GmbH
#*****************************************************************************
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

MAK_NAME=z17_z127_sw

MAK_SWITCH=$(SW_PREFIX)MAC_MEM_MAPPED \
			$(SW_PREFIX)MAC_BYTESWAP  \
			$(SW_PREFIX)Z17_VARIANT=Z17_Z127_SW \
			$(SW_PREFIX)Z17_MODEL_Z127 \

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
