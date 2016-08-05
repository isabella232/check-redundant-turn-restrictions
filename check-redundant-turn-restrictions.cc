#include <cstdlib>

#include <ios>
#include <locale>

#include <osmium/io/pbf_input.hpp>
#include <osmium/io/reader.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/visitor.hpp>

#include "oneways.h"
#include "restrictions.h"

//
// Check for redundant Turn Restrictions against a Oneways direction
//

int main(int argc, char **argv) try {
  std::locale::global(std::locale(""));
  std::ios_base::sync_with_stdio(false);

  if (argc != 2) {
    std::fprintf(stderr, "Usage: %s in.osm.pbf\n", argv[0]);
    return EXIT_FAILURE;
  }

  const auto entities = osmium::osm_entity_bits::nwr;
  osmium::io::Reader reader(argv[1], entities);

  OnewayHandler oneway;
  RestrictionHandler restriction;

  osmium::apply(reader, oneway, restriction);
  reader.close();

  const auto &turnRestrictions = restriction.restrictions;
  const auto &onewayStreets = oneway.oneways;

  std::fprintf(stderr, "Restrictions: %zu\n", turnRestrictions.size());
  std::fprintf(stderr, "Oneways: %zu\n", onewayStreets.size());

  const auto onewayNotFound = onewayStreets.end();

  for (const auto &restriction : turnRestrictions) {
    const Oneway way{restriction.to, restriction.via};

    const auto it = onewayStreets.find(way);

    if (it != onewayNotFound)
      std::printf("%zu (%zu, %zu, %zu)\n", //
                  restriction.id, restriction.from, restriction.via, restriction.to);
  }

} catch (const std::exception &e) {
  std::fprintf(stderr, "Error: %s\n", e.what());
  return EXIT_FAILURE;
}
