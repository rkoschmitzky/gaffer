//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"

#include "MonitorBinding.h"

#include "Gaffer/ContextMonitor.h"
#include "Gaffer/Monitor.h"
#include "Gaffer/MonitorAlgo.h"
#include "Gaffer/PerformanceMonitor.h"
#include "Gaffer/Plug.h"
#include "Gaffer/VTuneMonitor.h"

#include "boost/format.hpp"

using namespace boost::python;
using namespace Gaffer;
using namespace Gaffer::MonitorAlgo;

namespace
{

void enterScope( Monitor &m )
{
	m.setActive( true );
}

void exitScope( Monitor &m, object type, object value, object traceBack )
{
	m.setActive( false );
}

std::string repr( PerformanceMonitor::Statistics &s )
{
	return boost::str(
		boost::format( "Gaffer.PerformanceMonitor.Statistics( hashCount = %d, computeCount = %d, hashDuration = %d, computeDuration = %d )" )
			% s.hashCount
			% s.computeCount
			% s.hashDuration.count()
			% s.computeDuration.count()
	);
}

PerformanceMonitor::Statistics *statisticsConstructor(
	size_t hashCount,
	size_t computeCount,
	boost::chrono::nanoseconds::rep hashDuration,
	boost::chrono::nanoseconds::rep computeDuration
)
{
	return new PerformanceMonitor::Statistics( hashCount, computeCount, boost::chrono::nanoseconds( hashDuration ), boost::chrono::nanoseconds( computeDuration ) );
}

boost::chrono::nanoseconds::rep getHashDuration( PerformanceMonitor::Statistics &s )
{
	return s.hashDuration.count();
}

void setHashDuration( PerformanceMonitor::Statistics &s, boost::chrono::nanoseconds::rep v )
{
	s.hashDuration = boost::chrono::nanoseconds( v );
}

boost::chrono::nanoseconds::rep getComputeDuration( PerformanceMonitor::Statistics &s )
{
	return s.computeDuration.count();
}

void setComputeDuration( PerformanceMonitor::Statistics &s, boost::chrono::nanoseconds::rep v )
{
	s.computeDuration = boost::chrono::nanoseconds( v );
}

template<typename T>
dict allStatistics( T &m )
{
	dict result;
	const typename T::StatisticsMap &s = m.allStatistics();
	for( typename T::StatisticsMap::const_iterator it = s.begin(), eIt = s.end(); it != eIt; ++it )
	{
		result[boost::const_pointer_cast<Plug>( it->first)] = it->second;
	}
	return result;
}

list contextMonitorVariableNames( const ContextMonitor::Statistics &s )
{
	std::vector<IECore::InternedString> names = s.variableNames();
	list result;
	for( std::vector<IECore::InternedString>::const_iterator it = names.begin(), eIt = names.end(); it != eIt; ++it )
	{
		result.append( it->c_str() );
	}
	return result;
}

} // namespace

void GafferModule::bindMonitor()
{

	{
		object module( borrowed( PyImport_AddModule( "Gaffer.MonitorAlgo" ) ) );
		scope().attr( "MonitorAlgo" ) = module;
		scope moduleScope( module );

		enum_<PerformanceMetric>( "PerformanceMetric" )
			.value( "Invalid", Invalid )
			.value( "TotalDuration", TotalDuration )
			.value( "HashDuration", HashDuration )
			.value( "ComputeDuration", ComputeDuration )
			.value( "PerHashDuration", PerHashDuration )
			.value( "PerComputeDuration", PerComputeDuration )
			.value( "HashCount", HashCount )
			.value( "ComputeCount", ComputeCount )
			.value( "HashesPerCompute", HashesPerCompute )
		;

		def(
			"formatStatistics",
			( std::string (*)( const PerformanceMonitor &, size_t ) )&formatStatistics,
			(
				arg( "monitor" ),
				arg( "maxLinesPerMetric" ) = 50
			)
		);

		def(
			"formatStatistics",
			( std::string (*)( const PerformanceMonitor &, PerformanceMetric, size_t ) )&formatStatistics,
			(
				arg( "monitor" ),
				arg( "metric" ),
				arg( "maxLines" ) = 50
			)
		);
	}

	class_<Monitor, boost::noncopyable>( "Monitor", no_init )
		.def( "setActive", &Monitor::setActive )
		.def( "getActive", &Monitor::getActive )
		.def( "__enter__", &enterScope, return_self<>() )
		.def( "__exit__", &exitScope )
	;

	{
		scope s = class_<PerformanceMonitor, bases<Monitor>, boost::noncopyable >( "PerformanceMonitor" )
			.def( "allStatistics", &allStatistics<PerformanceMonitor> )
			.def( "plugStatistics", &PerformanceMonitor::plugStatistics, return_value_policy<copy_const_reference>() )
			.def( "combinedStatistics", &PerformanceMonitor::combinedStatistics, return_value_policy<copy_const_reference>() )
		;

		class_<PerformanceMonitor::Statistics>( "Statistics" )
			.def( "__init__", make_constructor( statisticsConstructor, default_call_policies(),
					(
						arg( "hashCount" ) = 0,
						arg( "computeCount" ) = 0,
						arg( "hashDuration" ) = 0,
						arg( "computeDuration" ) = 0
					)
				)
			)
			.def_readwrite( "hashCount", &PerformanceMonitor::Statistics::hashCount )
			.def_readwrite( "computeCount", &PerformanceMonitor::Statistics::computeCount )
			.add_property( "hashDuration", &getHashDuration, &setHashDuration )
			.add_property( "computeDuration", &getComputeDuration, &setComputeDuration )
			.def( self == self )
			.def( self != self )
			.def( "__repr__", &repr )
		;
	}

	{
		scope s = class_<ContextMonitor, bases<Monitor>, boost::noncopyable>( "ContextMonitor", no_init )
			.def( init<const GraphComponent *>( arg( "root" ) = object() ) )
			.def( "allStatistics", &allStatistics<ContextMonitor> )
			.def( "plugStatistics", &ContextMonitor::plugStatistics, return_value_policy<copy_const_reference>() )
			.def( "combinedStatistics", &ContextMonitor::combinedStatistics, return_value_policy<copy_const_reference>() )
		;

		class_<ContextMonitor::Statistics>( "Statistics" )
			.def( "numUniqueContexts", &ContextMonitor::Statistics::numUniqueContexts )
			.def( "variableNames", &contextMonitorVariableNames )
			.def( "numUniqueValues", &ContextMonitor::Statistics::numUniqueValues )
			.def( self == self )
			.def( self != self )
		;
	}

#ifdef GAFFER_VTUNE
	{
		scope s = class_<VTuneMonitor, bases<Monitor>, boost::noncopyable>( "VTuneMonitor" )
			.def( init<bool>(
					(
						arg( "monitorHashProcess" ) = false )
					)
				);
	}

#endif //GAFFER_VTUNE

}
