//
//  LODManager.cpp
//
//
//  Created by Clement on 1/16/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <SettingHandle.h>
#include <Util.h>

#include "Application.h"
#include "ui/DialogsManager.h"

#include "LODManager.h"

Setting::Handle<float> desktopLODDecreaseFPS("desktopLODDecreaseFPS", DEFAULT_DESKTOP_LOD_DOWN_FPS);
Setting::Handle<float> hmdLODDecreaseFPS("hmdLODDecreaseFPS", DEFAULT_HMD_LOD_DOWN_FPS);

LODManager::LODManager() {
    calculateAvatarLODDistanceMultiplier();
}

float LODManager::getLODDecreaseFPS() {
    if (Application::getInstance()->isHMDMode()) {
        return getHMDLODDecreaseFPS();
    }
    return getDesktopLODDecreaseFPS();
}

float LODManager::getLODIncreaseFPS() {
    if (Application::getInstance()->isHMDMode()) {
        return getHMDLODIncreaseFPS();
    }
    return getDesktopLODIncreaseFPS();
}


void LODManager::autoAdjustLOD(float currentFPS) {
    
    // NOTE: our first ~100 samples at app startup are completely all over the place, and we don't
    // really want to count them in our average, so we will ignore the real frame rates and stuff
    // our moving average with simulated good data
    const int IGNORE_THESE_SAMPLES = 100;
    //const float ASSUMED_FPS = 60.0f;
    if (_fpsAverageUpWindow.getSampleCount() < IGNORE_THESE_SAMPLES) {
        currentFPS = ASSUMED_FPS;
        _lastUpShift = _lastDownShift = usecTimestampNow();
    }
    _fpsAverageStartWindow.updateAverage(currentFPS);
    _fpsAverageDownWindow.updateAverage(currentFPS);
    _fpsAverageUpWindow.updateAverage(currentFPS);
    
    quint64 now = usecTimestampNow();

    bool changed = false;
    bool octreeChanged = false;
    quint64 elapsedSinceDownShift = now - _lastDownShift;
    quint64 elapsedSinceUpShift = now - _lastUpShift;
    
    if (_automaticLODAdjust) {
    
        // LOD Downward adjustment 
        // If our last adjust was an upshift, then we don't want to consider any downshifts until we've delayed at least
        // our START_DELAY_WINDOW_IN_SECS
        bool doDownShift = _lastAdjustWasUpShift
                ? (elapsedSinceUpShift > START_SHIFT_ELPASED && _fpsAverageStartWindow.getAverage() < getLODDecreaseFPS())
                : (elapsedSinceDownShift > DOWN_SHIFT_ELPASED && _fpsAverageDownWindow.getAverage() < getLODDecreaseFPS());

        
        if (doDownShift) {

            // Octree items... stepwise adjustment
            if (_octreeSizeScale > ADJUST_LOD_MIN_SIZE_SCALE) {
                _octreeSizeScale *= ADJUST_LOD_DOWN_BY;
                if (_octreeSizeScale < ADJUST_LOD_MIN_SIZE_SCALE) {
                    _octreeSizeScale = ADJUST_LOD_MIN_SIZE_SCALE;
                }
                octreeChanged = changed = true;
            }

            if (changed) {
                if (_lastAdjustWasUpShift) {
                    qDebug() << "adjusting LOD DOWN after initial delay..."
                                << "average fps for last "<< START_DELAY_WINDOW_IN_SECS <<"seconds was " 
                                << _fpsAverageStartWindow.getAverage() 
                                << "minimum is:" << getLODDecreaseFPS() 
                                << "elapsedSinceUpShift:" << elapsedSinceUpShift
                                << " NEW _octreeSizeScale=" << _octreeSizeScale;
                } else {
                    qDebug() << "adjusting LOD DOWN..."
                                << "average fps for last "<< DOWN_SHIFT_WINDOW_IN_SECS <<"seconds was " 
                                << _fpsAverageDownWindow.getAverage() 
                                << "minimum is:" << getLODDecreaseFPS() 
                                << "elapsedSinceDownShift:" << elapsedSinceDownShift
                                << " NEW _octreeSizeScale=" << _octreeSizeScale;
                }

                _lastDownShift = now;
                _lastAdjustWasUpShift = false;
        
                emit LODDecreased();
            }
        } else {
    
            // LOD Upward adjustment
            if (elapsedSinceUpShift > UP_SHIFT_ELPASED && _fpsAverageUpWindow.getAverage() > getLODIncreaseFPS()) {

                // Octee items... stepwise adjustment
                if (_octreeSizeScale < ADJUST_LOD_MAX_SIZE_SCALE) {
                    if (_octreeSizeScale < ADJUST_LOD_MIN_SIZE_SCALE) {
                        _octreeSizeScale = ADJUST_LOD_MIN_SIZE_SCALE;
                    } else {
                        _octreeSizeScale *= ADJUST_LOD_UP_BY;
                    }
                    if (_octreeSizeScale > ADJUST_LOD_MAX_SIZE_SCALE) {
                        _octreeSizeScale = ADJUST_LOD_MAX_SIZE_SCALE;
                    }
                    octreeChanged = changed = true;
                }
        
                if (changed) {
                    qDebug() << "adjusting LOD UP... average fps for last "<< UP_SHIFT_WINDOW_IN_SECS <<"seconds was " 
                                << _fpsAverageUpWindow.getAverage()
                                << "upshift point is:" << getLODIncreaseFPS() 
                                << "elapsedSinceUpShift:" << elapsedSinceUpShift
                                << " NEW _octreeSizeScale=" << _octreeSizeScale;

                    _lastUpShift = now;
                    _lastAdjustWasUpShift = true;
                
                    emit LODIncreased();
                }
            }
        }
    
        if (changed) {
            calculateAvatarLODDistanceMultiplier();
            _shouldRenderTableNeedsRebuilding = true;
            auto lodToolsDialog = DependencyManager::get<DialogsManager>()->getLodToolsDialog();
            if (lodToolsDialog) {
                lodToolsDialog->reloadSliders();
            }
        }
    }
}

void LODManager::resetLODAdjust() {

    // TODO: Do we need this???
    /*
    _fpsAverageDownWindow.reset();
    _fpsAverageUpWindow.reset();
    _lastAdjust = usecTimestampNow();
    */
}

QString LODManager::getLODFeedbackText() {
    // determine granularity feedback
    int boundaryLevelAdjust = getBoundaryLevelAdjust();
    QString granularityFeedback;
    
    switch (boundaryLevelAdjust) {
        case 0: {
            granularityFeedback = QString(".");
        } break;
        case 1: {
            granularityFeedback = QString(" at half of standard granularity.");
        } break;
        case 2: {
            granularityFeedback = QString(" at a third of standard granularity.");
        } break;
        default: {
            granularityFeedback = QString(" at 1/%1th of standard granularity.").arg(boundaryLevelAdjust + 1);
        } break;
    }
    
    // distance feedback
    float octreeSizeScale = getOctreeSizeScale();
    float relativeToDefault = octreeSizeScale / DEFAULT_OCTREE_SIZE_SCALE;
    int relativeToTwentyTwenty = 20 / relativeToDefault;

    QString result;
    if (relativeToDefault > 1.01) {
        result = QString("20:%1 or %2 times further than average vision%3").arg(relativeToTwentyTwenty).arg(relativeToDefault,0,'f',2).arg(granularityFeedback);
    } else if (relativeToDefault > 0.99) {
        result = QString("20:20 or the default distance for average vision%1").arg(granularityFeedback);
    } else if (relativeToDefault > 0.01) {
        result = QString("20:%1 or %2 of default distance for average vision%3").arg(relativeToTwentyTwenty).arg(relativeToDefault,0,'f',3).arg(granularityFeedback);
    } else {
        result = QString("%2 of default distance for average vision%3").arg(relativeToDefault,0,'f',3).arg(granularityFeedback);
    }
    return result;
}

// TODO: This is essentially the same logic used to render octree cells, but since models are more detailed then octree cells
//       I've added a voxelToModelRatio that adjusts how much closer to a model you have to be to see it.
bool LODManager::shouldRenderMesh(float largestDimension, float distanceToCamera) {
    const float octreeToMeshRatio = 4.0f; // must be this many times closer to a mesh than a voxel to see it.
    float octreeSizeScale = getOctreeSizeScale();
    int boundaryLevelAdjust = getBoundaryLevelAdjust();
    float maxScale = (float)TREE_SCALE;
    float visibleDistanceAtMaxScale = boundaryDistanceForRenderLevel(boundaryLevelAdjust, octreeSizeScale) / octreeToMeshRatio;
    
    if (_shouldRenderTableNeedsRebuilding) {
        _shouldRenderTable.clear();
        
        float SMALLEST_SCALE_IN_TABLE = 0.001f; // 1mm is plenty small
        float scale = maxScale;
        float visibleDistanceAtScale = visibleDistanceAtMaxScale;
        
        while (scale > SMALLEST_SCALE_IN_TABLE) {
            scale /= 2.0f;
            visibleDistanceAtScale /= 2.0f;
            _shouldRenderTable[scale] = visibleDistanceAtScale;
        }
        _shouldRenderTableNeedsRebuilding = false;
    }
    
    float closestScale = maxScale;
    float visibleDistanceAtClosestScale = visibleDistanceAtMaxScale;
    QMap<float, float>::const_iterator lowerBound = _shouldRenderTable.lowerBound(largestDimension);
    if (lowerBound != _shouldRenderTable.constEnd()) {
        closestScale = lowerBound.key();
        visibleDistanceAtClosestScale = lowerBound.value();
    }
    
    if (closestScale < largestDimension) {
        visibleDistanceAtClosestScale *= 2.0f;
    }
    
    return (distanceToCamera <= visibleDistanceAtClosestScale);
}

void LODManager::setOctreeSizeScale(float sizeScale) {
    _octreeSizeScale = sizeScale;
    calculateAvatarLODDistanceMultiplier();
    _shouldRenderTableNeedsRebuilding = true;
}

void LODManager::calculateAvatarLODDistanceMultiplier() {
    _avatarLODDistanceMultiplier = AVATAR_TO_ENTITY_RATIO / (_octreeSizeScale / DEFAULT_OCTREE_SIZE_SCALE);
}

void LODManager::setBoundaryLevelAdjust(int boundaryLevelAdjust) {
    _boundaryLevelAdjust = boundaryLevelAdjust;
    _shouldRenderTableNeedsRebuilding = true;
}


void LODManager::loadSettings() {
    setDesktopLODDecreaseFPS(desktopLODDecreaseFPS.get());
    setHMDLODDecreaseFPS(hmdLODDecreaseFPS.get());
}

void LODManager::saveSettings() {
    desktopLODDecreaseFPS.set(getDesktopLODDecreaseFPS());
    hmdLODDecreaseFPS.set(getHMDLODDecreaseFPS());
}


