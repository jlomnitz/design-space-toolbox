/**
 * \file DSNVertexEnumeration.h
 * \brief
 *
 * \details
 *
 * Copyright (C) 2011-2014 Jason Lomnitz.\n\n
 *
 * This file is part of the Design Space Toolbox V2 (C Library).
 *
 * The Design Space Toolbox V2 is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Design Space Toolbox V2 is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Design Space Toolbox. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * \author Jason Lomnitz.
 * \date 2014
 *
 */

#ifndef DesignSpaceToolboxV2_DSNVertexEnumeration_h
#define DesignSpaceToolboxV2_DSNVertexEnumeration_h

DSMatrixArray * DSCaseNDVertexEnumeration(const DSCase *aCase, const DSVariablePool * lowerBounds, const DSVariablePool *upperBounds);



#endif
