// CCMCallback.as - ActionScript for System Menu callback handling
// Runs on frame 1, hooks onCategoryButtonPress to handle CCM entry clicks

import skse;

// SystemFader is a child of Menu_mc, not QuestJournalFader
var systemPage = _parent.SystemFader.Page_mc;

if (systemPage != undefined) {
    var categoryList = systemPage.CategoryList_mc.List_mc;
    
    if (categoryList != undefined) {
        // Find our entry's index (marked with isCCM flag by C++ injection)
        var ccmIndex = -1;
        var entryList = categoryList.entryList;
        for (var i = 0; i < entryList.length; i++) {
            if (entryList[i].isCCM == true) {
                ccmIndex = i;
                break;
            }
        }
        
        if (ccmIndex >= 0) {
            // Store original callback and our index
            systemPage.CCM_originalOnCategoryButtonPress = systemPage.onCategoryButtonPress;
            systemPage.CCM_insertIndex = ccmIndex;
            
            // Replace with our wrapper
            systemPage.onCategoryButtonPress = function(event) {
                if (event.entry.isCCM == true) {
                    // Our entry was clicked - send mod event
                    skse.SendModEvent('CCMUI_Open');
                } else {
                    // Adjust index for items after our insertion
                    if (this.CCM_insertIndex >= 0 && event.index > this.CCM_insertIndex) {
                        event.index--;
                    }
                    // Call original handler
                    if (this.CCM_originalOnCategoryButtonPress != undefined) {
                        this.CCM_originalOnCategoryButtonPress(event);
                    }
                }
            };
        }
    }
} else {
    // Fallback: poll for SystemFader in case it loads late
    _root.CCM_checkCount = 0;
    this.onEnterFrame = function() {
        _root.CCM_checkCount++;
        if (_root.CCM_checkCount > 60) {
            delete this.onEnterFrame;
            return;
        }
        
        var sp = _parent.SystemFader.Page_mc;
        if (sp != undefined) {
            var cl = sp.CategoryList_mc.List_mc;
            if (cl != undefined) {
                var idx = -1;
                for (var i = 0; i < cl.entryList.length; i++) {
                    if (cl.entryList[i].isCCM == true) {
                        idx = i;
                        break;
                    }
                }
                
                if (idx >= 0) {
                    sp.CCM_originalOnCategoryButtonPress = sp.onCategoryButtonPress;
                    sp.CCM_insertIndex = idx;
                    sp.onCategoryButtonPress = function(event) {
                        if (event.entry.isCCM == true) {
                            skse.SendModEvent('CCMUI_Open');
                        } else {
                            if (this.CCM_insertIndex >= 0 && event.index > this.CCM_insertIndex) {
                                event.index--;
                            }
                            if (this.CCM_originalOnCategoryButtonPress != undefined) {
                                this.CCM_originalOnCategoryButtonPress(event);
                            }
                        }
                    };
                }
            }
            delete this.onEnterFrame;
        }
    };
}
