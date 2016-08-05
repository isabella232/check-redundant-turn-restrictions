## check-redundant-turn-restrictions

Checks for redundant (fromWay, viaNode, toWay) turn restrictions constraining a turn into a oneway's opposite direction.

## Building

Requires Boost for its `hash_combine` feature.
A recent libosmium is getting downloaded to `third_party` by invoking the `./deps.sh` script.

    ./deps.sh
    mkdir build && cd build
    cmake ..
    cmake --build .

## Running

    ./check-redundant-turn-restrictions planet.osm.pbf

Output format on `stdout` is:

    RelationId (FromWayId, ViaNodeId, ToWayId)

All other output goes to `stderr`.

## License

Copyright © 2016 Mapbox

Distributed under the MIT License (MIT).
