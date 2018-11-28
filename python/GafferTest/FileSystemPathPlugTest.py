##########################################################################
#
#  Copyright (c) 2012, John Haddon. All rights reserved.
#  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#      * Redistributions of source code must retain the above
#        copyright notice, this list of conditions and the following
#        disclaimer.
#
#      * Redistributions in binary form must reproduce the above
#        copyright notice, this list of conditions and the following
#        disclaimer in the documentation and/or other materials provided with
#        the distribution.
#
#      * Neither the name of John Haddon nor the names of
#        any other contributors to this software may be used to endorse or
#        promote products derived from this software without specific prior
#        written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##########################################################################

import os
import inspect
import unittest

import IECore

import Gaffer
import GafferTest

class FileSystemPathPlugTest( GafferTest.TestCase ) :

	def testGenericToWindows( self ) :
		if os.name == "nt":
			n = GafferTest.FileSystemPathInOutNode()
			self.assertHashesValid( n )

			# generic to Windows should preserve drive letters
			n["in"].setValue( "C:/path/test.exr" )
			self.assertEqual( n["out"].getValue(), "C:\\path\\test.exr" )

			# generic starting with a single forward slash is interpreted
			# as a UNC path and should start with double back slash

			n["in"].setValue( "/test.server/path/test.exr" )
			self.assertEqual( n["out"].getValue(), "\\\\test.server\\path\\test.exr" )

	def testGenericToPosix( self ):
		if os.name != "nt":
			n = GafferTest.FileSystemPathInOutNode()
			self.assertHashesValid( n )

			# generic to Posix should return unchanged
			n["in"].setValue( "/path/test.exr" )
			self.assertEqual( n["out"].getValue(), "/path/test.exr" )

			n["in"].setValue( "/test.server/path/test.exr" )
			self.assertEqual( n["out"].getValue(), "/test.server/path/test.exr" )

			# TODO: what should a drive letter path return on POSIX?
			# Error? 'C:' -> '/C/'?

	def testTildeExpansion( self ) :

		n = GafferTest.StringInOutNode()

		n["in"].setValue( "~" )
		self.assertEqual( n["out"].getValue(), os.path.expanduser( "~" ) )

		n["in"].setValue( "~/something.tif" )
		self.assertEqual( n["out"].getValue(), os.path.expanduser( "~/something.tif" ) )

		# ~ shouldn't be expanded unless it's at the front - it would
		# be meaningless in other cases.
		n["in"].setValue( "in ~1900" )
		self.assertEqual( n["out"].getValue(), "in ~1900" )

	def testEnvironmentExpansion( self ) :

		n = GafferTest.StringInOutNode()

		n["in"].setValue( "${NOT_AN_ENVIRONMENT_VARIABLE}" )
		h1 = n["out"].hash()
		self.assertEqual( n["out"].getValue(), "" )

		n["in"].setValue( "${GAFFER_ROOT}" )
		self.assertEqual( n["out"].getValue(), os.environ["GAFFER_ROOT"] )
		h2 = n["out"].hash()
		self.assertNotEqual( h1, h2 )

		context = Gaffer.Context()
		context["GAFFER_ROOT"] = "b"
		with context :
			# context should win against environment
			self.assertEqual( n["out"].getValue(), "b" )
			self.assertNotEqual( n["out"].hash(), h1 )
			self.assertNotEqual( n["out"].hash(), h2 )

	def testSubstitutionsFromExpressionInput( self ) :

		s = Gaffer.ScriptNode()

		# Should output a substituted version of the input.
		s["substitionsOn"] = GafferTest.StringInOutNode()

		# Should pass through the input directly, without substitutions.
		s["substitionsOff"] = GafferTest.StringInOutNode( substitutions = Gaffer.Context.Substitutions.NoSubstitutions )

		# The third case is trickier. The "in" plug on the node
		# itself requests no substitutions, but it receives its
		# input via an indirect connection with substitutions
		# turned on. We resolve this by defining substitutions
		# to occur only when observing a value inside a compute,
		# and to always be determined by the plug used to access
		# the value. A chain of connections can be thought of as
		# carrying an unsubstituted string all the way along
		# internally, with each plug along the way determining
		# the substitutions applied when peeking in to see the value
		# at that point.
		#
		# In practice this works best because typically it is only
		# nodes that know when a substitution is relevant, and the
		# user shouldn't be burdened with the job of thinking about
		# them when making intermediate connections to that node.
		s["substitionsOnIndirectly"] = GafferTest.StringInOutNode( substitutions = Gaffer.Context.Substitutions.NoSubstitutions )
		s["substitionsOnIndirectly"]["user"]["in"] = Gaffer.StringPlug()
		s["substitionsOnIndirectly"]["in"].setInput( s["substitionsOnIndirectly"]["user"]["in"] )

		# All three nodes above receive their input from this expression
		# which outputs a sequence value to be substituted (or not).

		s["e"] = Gaffer.Expression()
		s["e"].setExpression( inspect.cleandoc(
			"""
			parent["substitionsOn"]["in"] = "test.#.exr"
			parent["substitionsOff"]["in"] = "test.#.exr"
			parent["substitionsOnIndirectly"]["user"]["in"] = "test.#.exr"
			"""
		) )

		with Gaffer.Context() as c :

			# Frame 1
			#########

			c.setFrame( 1 )

			# The output of the expression itself is not substituted.
			# Substitutions occur only on input plugs.

			self.assertEqual( s["substitionsOn"]["in"].getInput().getValue(), "test.#.exr" )
			self.assertEqual( s["substitionsOff"]["in"].getInput().getValue(), "test.#.exr" )
			self.assertEqual( s["substitionsOnIndirectly"]["user"]["in"].getInput().getValue(), "test.#.exr" )

			# We should get frame numbers out of the substituting node.

			self.assertEqual( s["substitionsOn"]["out"].getValue(), "test.1.exr" )
			substitutionsOnHash1 = s["substitionsOn"]["out"].hash()
			self.assertEqual( s["substitionsOn"]["out"].getValue( _precomputedHash = substitutionsOnHash1 ), "test.1.exr" )

			# We should get sequences out of the non-substituting node.

			self.assertEqual( s["substitionsOff"]["out"].getValue(), "test.#.exr" )
			substitutionsOffHash1 = s["substitionsOff"]["out"].hash()
			self.assertEqual( s["substitionsOff"]["out"].getValue( _precomputedHash = substitutionsOffHash1 ), "test.#.exr" )
			self.assertNotEqual( substitutionsOnHash1, substitutionsOffHash1 )

			# We shouldn't get frame numbers out of the third node, because the
			# requirements of the node (no substitutions) trump any upstream opinions.
			# Substitutions are performed by the plug during value access, and do not
			# affect the actual data flow.

			self.assertEqual( s["substitionsOnIndirectly"]["out"].getValue(), "test.#.exr" )
			substitionsOnIndirectlyHash1 = s["substitionsOnIndirectly"]["out"].hash()
			self.assertEqual( s["substitionsOnIndirectly"]["out"].getValue( _precomputedHash = substitionsOnIndirectlyHash1 ), "test.#.exr" )

			# Frame 2
			#########

			c.setFrame( 2 )

			# The output of the expression itself is not substituted.
			# Substitutions occur only on input plugs.

			self.assertEqual( s["substitionsOn"]["in"].getInput().getValue(), "test.#.exr" )
			self.assertEqual( s["substitionsOff"]["in"].getInput().getValue(), "test.#.exr" )
			self.assertEqual( s["substitionsOnIndirectly"]["user"]["in"].getInput().getValue(), "test.#.exr" )

			# We should get frame numbers out of the substituting node.
			# The hash must has changed to make this possible.

			self.assertEqual( s["substitionsOn"]["out"].getValue(), "test.2.exr" )
			substitutionsOnHash2 = s["substitionsOn"]["out"].hash()
			self.assertEqual( s["substitionsOn"]["out"].getValue( _precomputedHash = substitutionsOnHash2 ), "test.2.exr" )
			self.assertNotEqual( substitutionsOnHash2, substitutionsOnHash1 )

			# We should still get sequences out of the non-substituting node,
			# and it should have the same output hash as it had on frame 1.

			self.assertEqual( s["substitionsOff"]["out"].getValue(), "test.#.exr" )
			substitutionsOffHash2 = s["substitionsOff"]["out"].hash()
			self.assertEqual( s["substitionsOff"]["out"].getValue( _precomputedHash = substitutionsOffHash2 ), "test.#.exr" )
			self.assertEqual( substitutionsOffHash1, substitutionsOffHash2 )
			self.assertNotEqual( substitutionsOnHash2, substitutionsOffHash2 )

			# The third node should still be non-substituting.

			self.assertEqual( s["substitionsOnIndirectly"]["out"].getValue(), "test.#.exr" )
			substitionsOnIndirectlyHash2 = s["substitionsOnIndirectly"]["out"].hash()
			self.assertEqual( s["substitionsOnIndirectly"]["out"].getValue( _precomputedHash = substitionsOnIndirectlyHash2 ), "test.#.exr" )
			self.assertEqual( substitionsOnIndirectlyHash2, substitionsOnIndirectlyHash1 )



if __name__ == "__main__":
	unittest.main()
