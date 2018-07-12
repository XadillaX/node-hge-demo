{
  "targets": [{
    "target_name": "node_hge",
    "sources": [
      "src/entry.cpp"
    ],
    "include_dirs": [
      "src/hge181/include",
      "<!(node -e \"require('nan')\")"
    ],
    "libraries": [
      "../src/hge181/lib/vc/hge.lib",
      "../src/hge181/lib/vc/hgehelp.lib"
    ],
    "libraries!": [
      "libc.lib"
    ],
    "defines": [
      "WIN32_LEAN_AND_MEAN"
    ],
    "VCLinkerTool": {
      "IgnoreSpecificDefaultLibraries": [
        "libc.lib"
      ]
    },
    "copies": [{
      "destination": "<(module_root_dir)/build/Release/",
      "files": [ "<(module_root_dir)/src/hge181/hge.dll", "<(module_root_dir)/src/hge181/bass.dll" ]
    }]
  }]
}