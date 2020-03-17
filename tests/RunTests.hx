package;

import tink.testrunner.*;
import tink.unit.*;
import tink.unit.Assert.assert;
import sys.FileSystem;
import Utils.attempt;
import Utils.shouldFail;
import synch.*;

using Utils;
using tink.CoreApi;
using Lambda;

class RunTests {
	static function main() {
		// final tests:Array<BasicTest> = [];
		Runner.run(TestBatch.make([new BasicTest()])).handle(Runner.exit);

	}
}

@:asserts
class BasicTest {
	public function new() {}

	var FILE_SIZE:haxe.Int64 = haxe.Int64.fromFloat(Math.pow(2, 16));
	var mapping:mem.FileMapping;
	var view:mem.MapView;

	public function testCreateFileMapping() {
		return assert(attempt({
			mapping = mem.FileMapping.create("./test.db", "test-map", FILE_SIZE);
		}));
	}

	public function testCreateFileView() {
		return assert(attempt({
			view = mapping.createView(0, Std.int(Math.pow(2, 13)) );
		}));
	}

	public function testWriteToView() {
		return assert(attempt({

			view.write(haxe.io.Bytes.ofString("some data"));
		}));
	}

	public function testPersistToView() {
		return assert(attempt({

			view.persist();
		}));
	}

	public function testReadView() {
		return assert(attempt(view.read().toString() == 'some data'));
	}
}
