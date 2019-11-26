#***************************  M a k e f i l e  *******************************
#
#         Author: ub
#
#    Description: Makefile definitions for the Z17 example program
#
#-----------------------------------------------------------------------------
#   Copyright 2000-2019, MEN Mikro Elektronik GmbH
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

MAK_NAME=z17_simp
# the next line is updated during the MDIS installation
STAMPED_REVISION="13Z017-06_01_30-5-g0d277bf-dirty_2019-05-28"

DEF_REVISION=MAK_REVISION=$(STAMPED_REVISION)
MAK_SWITCH=$(SW_PREFIX)$(DEF_REVISION)

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)	\
			$(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)	\

MAK_INCL=$(MEN_INC_DIR)/z17_drv.h	\
         $(MEN_INC_DIR)/men_typs.h	\
         $(MEN_INC_DIR)/mdis_api.h	\
         $(MEN_INC_DIR)/usr_oss.h	\

MAK_INP1=z17_simp$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
