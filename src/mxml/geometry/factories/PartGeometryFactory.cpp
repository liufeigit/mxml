//  Created by Alejandro Isaza on 2014-12-18.
//  Copyright (c) 2014 Venture Media Labs. All rights reserved.

#include "DirectionGeometryFactory.h"
#include "EndingGeometryFactory.h"
#include "LyricGeometryFactory.h"
#include "OrnamentGeometryFactory.h"
#include "PartGeometryFactory.h"
#include "TieGeometryFactory.h"

#include <mxml/geometry/collisions/CollisionHandler.h>


namespace mxml {

PartGeometryFactory::PartGeometryFactory(const dom::Part& part, const ScoreProperties& scoreProperties, const ScrollMetrics& metrics, const SpanCollection& spans)
: _part(part),
  _scoreProperties(scoreProperties),
  _metrics(metrics),
  _spans(spans),
  _partGeometry()
{
}

PartGeometryFactory::~PartGeometryFactory() {
    assert(!_partGeometry);
}

std::unique_ptr<PartGeometry> PartGeometryFactory::build() {
    _partGeometry.reset(new PartGeometry(_part, _scoreProperties, _metrics));

    coord_t offset = 0;
    for (auto& measure : _part.measures()) {
        std::unique_ptr<MeasureGeometry> geo(new MeasureGeometry(*measure, _spans, _scoreProperties, _metrics));
        geo->setHorizontalAnchorPointValues(0, 0);
        geo->setLocation({offset, 0});
        offset += geo->size().width;

        _partGeometry->_measureGeometries.push_back(geo.get());
        _partGeometry->addGeometry(std::move(geo));
    }

    DirectionGeometryFactory directionGeometryFactory(_partGeometry.get(), _partGeometry->measureGeometries(), _metrics);
    auto directions = directionGeometryFactory.build();
    for (auto& geometry : directions) {
        _partGeometry->directionGeometries().push_back(geometry.get());
        _partGeometry->addGeometry(std::move(geometry));
    }

    OrnamentGeometryFactory ornamentGeometryFactory(_partGeometry.get(), _partGeometry->measureGeometries(), _metrics);
    auto ornaments = ornamentGeometryFactory.build();
    for (auto& geometry : ornaments) {
        _partGeometry->directionGeometries().push_back(geometry.get());
        _partGeometry->addGeometry(std::move(geometry));
    }

    EndingGeometryFactory endingnGeometryFactory(_partGeometry->measureGeometries(), _metrics);
    auto endings = endingnGeometryFactory.build();
    for (auto& geometry : endings) {
        _partGeometry->directionGeometries().push_back(geometry.get());
        _partGeometry->addGeometry(std::move(geometry));
    }

    LyricGeometryFactory lyricGeometryFactory(_partGeometry->measureGeometries(), _metrics);
    auto lyrics = lyricGeometryFactory.build();
    for (auto& lyric : lyrics) {
        _partGeometry->directionGeometries().push_back(lyric.get());
        _partGeometry->addGeometry(std::move(lyric));
    }

    TieGeometryFactory factory(*_partGeometry, _metrics);
    auto ties = factory.buildTieGeometries(_partGeometry->geometries());
    for (auto& tie : ties) {
        _partGeometry->_tieGeometries.push_back(tie.get());
        _partGeometry->addGeometry(std::move(tie));
    }
    
    CollisionHandler collisionHandler(*_partGeometry, _metrics);
    collisionHandler.resolveCollisions();
    
    _partGeometry->setBounds(_partGeometry->subGeometriesFrame());

    return std::move(_partGeometry);
}

} // namespace mxml