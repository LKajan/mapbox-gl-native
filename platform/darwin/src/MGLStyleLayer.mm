#import "MGLStyleLayer_Private.h"
#import "MGLMapView_Private.h"

#include <mbgl/map/map.hpp>
#include <mbgl/style/layer.hpp>

@interface MGLStyleLayer ()

@property (nonatomic, readonly) mbgl::style::Layer *rawLayer;

@end

@implementation MGLStyleLayer {
    std::unique_ptr<mbgl::style::Layer> _pendingLayer;
}

- (instancetype)initWithRawLayer:(mbgl::style::Layer *)rawLayer {
    if (self = [super init]) {
        _identifier = @(rawLayer->getID().c_str());
        _rawLayer = rawLayer;
    }
    return self;
}

- (instancetype)initWithPendingLayer:(std::unique_ptr<mbgl::style::Layer>)pendingLayer {
    if (self = [self initWithRawLayer:pendingLayer.get()]) {
        _pendingLayer = std::move(pendingLayer);
    }
    return self;
}

- (void)addToMapView:(MGLMapView *)mapView belowLayer:(MGLStyleLayer *)otherLayer
{
    if (_pendingLayer == nullptr) {
        [NSException raise:@"MGLRedundantLayerException"
            format:@"This instance %@ was already added to %@. Adding the same layer instance " \
                    "to the style more than once is invalid.", self, mapView.style];
    }

    if (otherLayer) {
        const mbgl::optional<std::string> belowLayerId{otherLayer.identifier.UTF8String};
        mapView.mbglMap->addLayer(std::move(_pendingLayer), belowLayerId);
    } else {
        mapView.mbglMap->addLayer(std::move(_pendingLayer));
    }
}

- (void)removeFromMapView:(MGLMapView *)mapView
{
    if (self.rawLayer == mapView.mbglMap->getLayer(self.identifier.UTF8String)) {
        _pendingLayer = mapView.mbglMap->removeLayer(self.identifier.UTF8String);
    }
}

- (void)setVisible:(BOOL)visible
{
    MGLAssertStyleLayerIsValid();

    mbgl::style::VisibilityType v = visible
    ? mbgl::style::VisibilityType::Visible
    : mbgl::style::VisibilityType::None;
    self.rawLayer->setVisibility(v);
}

- (BOOL)isVisible
{
    MGLAssertStyleLayerIsValid();

    mbgl::style::VisibilityType v = self.rawLayer->getVisibility();
    return (v == mbgl::style::VisibilityType::Visible);
}

- (void)setMaximumZoomLevel:(float)maximumZoomLevel
{
    MGLAssertStyleLayerIsValid();

    self.rawLayer->setMaxZoom(maximumZoomLevel);
}

- (float)maximumZoomLevel
{
    MGLAssertStyleLayerIsValid();

    return self.rawLayer->getMaxZoom();
}

- (void)setMinimumZoomLevel:(float)minimumZoomLevel
{
    MGLAssertStyleLayerIsValid();

    self.rawLayer->setMinZoom(minimumZoomLevel);
}

- (float)minimumZoomLevel
{
    MGLAssertStyleLayerIsValid();

    return self.rawLayer->getMinZoom();
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p; identifier = %@; visible = %@>",
            NSStringFromClass([self class]), (void *)self, self.identifier,
            self.visible ? @"YES" : @"NO"];
}

@end
