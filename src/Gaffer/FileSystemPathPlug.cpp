//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2012, John Haddon. All rights reserved.
//  Copyright (c) 2011-2015, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//
//      * Neither the name of John Haddon nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "Gaffer/FileSystemPathPlug.h"
#include "Gaffer/FileSystemPath.h"

#include "Gaffer/Context.h"
#include "Gaffer/Process.h"

using namespace IECore;
using namespace Gaffer;

IE_CORE_DEFINERUNTIMETYPED( FileSystemPathPlug );

FileSystemPathPlug::FileSystemPathPlug(
	const std::string &name,
	Direction direction,
	const std::string &defaultValue,
	unsigned flags,
	unsigned substitutions
)
	:	StringPlug( name, direction, defaultValue, flags ), m_substitutions( substitutions )
{
}

FileSystemPathPlug::~FileSystemPathPlug()
{
}

bool FileSystemPathPlug::acceptsInput( const Plug *input ) const
{
	if( !ValuePlug::acceptsInput( input ) )
	{
		return false;
	}
	if( input )
	{
		return input->isInstanceOf( StringPlug::staticTypeId() );
	}
	return true;
}

PlugPtr FileSystemPathPlug::createCounterpart( const std::string &name, Direction direction ) const
{
	return new FileSystemPathPlug( name, direction, defaultValue(), getFlags(), substitutions() );
}

std::string FileSystemPathPlug::getValue( const IECore::MurmurHash *precomputedHash ) const
{
	IECore::ConstObjectPtr o = getObjectValue( precomputedHash );
	const IECore::StringData *s = IECore::runTimeCast<const IECore::StringData>( o.get() );
	if( !s )
	{
		throw IECore::Exception( "StringPlug::getObjectValue() didn't return StringData - is the hash being computed correctly?" );
	}

	const bool performSubstitutions =
		m_substitutions &&
		direction() == In &&
		Process::current() &&
		Context::hasSubstitutions( s->readable() )
	;
	return performSubstitutions ? Context::current()->substitute(s->readable(), m_substitutions) : s->readable();
	//Gaffer::FileSystemPath p = Gaffer::FileSystemPath(sub_p);
	//return p.string();
	//return sub_p;
}
