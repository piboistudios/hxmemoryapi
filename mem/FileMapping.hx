package mem;
import mem.MemLib;
using mem.ErrorTools;
class FileMapping {
    public var fileName(default, null):String;
    public var mapName(default, null):String;
    public var mapSize(default, null):haxe.Int64;
    var mapHandle:LibFileMapping;
    function new(fileName, mapName, mapSize, mapHandle) {
        this.fileName = fileName;
        this.mapName = mapName;
        this.mapSize = mapSize;
        this.mapHandle = mapHandle;
    }
    static function checkErrors(mapHandle:LibFileMapping) {
        if(mapHandle.mapping_errored())
            throw mapHandle.mapping_get_errors().asErrorMsg();
    }
    public static function create(fileName:String, mapName:String, mapSize:haxe.Int64):mem.FileMapping {
        final handle:LibFileMapping = MemLib.create_file_mapping(fileName, mapName, mapSize.high, mapSize.low);
        checkErrors(handle);
        return new FileMapping(fileName, mapName, mapSize, handle);
    }
    public static function open(mapName:String):mem.FileMapping {
        final handle:LibFileMapping = MemLib.open_file_mapping(mapName);
        checkErrors(handle);
        return new FileMapping(null, mapName, -1, handle);
    }
    public function createView(offset:haxe.Int64, size:Null<UInt> = null):mem.MapView  @:privateAccess {
        if(size == null) size = mem.MemLib.get_sys_granularity();
        final handle:LibMapView = mapHandle.create_view(offset.high, offset.low, size);
        mem.MapView.checkErrors(handle);
        return new mem.MapView(offset, size, handle);
    }
}