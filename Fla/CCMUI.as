import skse;

class CCMUI extends MovieClip {

    public static var instance;

    public var SystemPage:MovieClip;
    public var CategoryList:MovieClip;

    private var LoadGameIndex = 0;
    private var itemEmbedDone:Boolean = false;

    function CCMUI() {
        CCMUI.instance = this;
        skse.SendModEvent('CCMUI_Debug', 'Constructor called');
    }

    function onLoad() {
        skse.SendModEvent('CCMUI_Debug', 'onLoad called');
        
        SystemPage = _parent._parent.SystemFader.Page_mc;
        CategoryList = SystemPage.CategoryList_mc.List_mc;

        duckPunch();
        InvalidateData();
    }

    function duckPunch() {
        CategoryList.CCM__InvalidateData = CategoryList.InvalidateData;
        CategoryList.InvalidateData = InvalidateData;

        SystemPage.CCM__onCategoryButtonPress = SystemPage.onCategoryButtonPress;
        SystemPage.onCategoryButtonPress = onCategoryButtonPress;
    }

    function InvalidateData() {
        this = CCMUI.instance;
        if ( itemEmbedDone === false ) {
            var entryList = CategoryList.entryList;
            for ( var i = 0; i < entryList.length; i++ ) {
                if ( entryList[ i ].text === '$LOAD' ) {
                    LoadGameIndex = i + 1;
                    break;
                }
            }
            CategoryList.entryList = entryList.slice(0, LoadGameIndex).concat( [ {
                text : 'Cheat Codes',
                isCCM : true
            } ], entryList.slice( LoadGameIndex ) );
            itemEmbedDone = true;
        }
        CategoryList.CCM__InvalidateData();
    }

    function onCategoryButtonPress(event:Object) {
        this = CCMUI.instance;
        if ( event.entry.isCCM ) {
            skse.SendModEvent('CCMUI_Open');
        } else {
            if ( event.index > LoadGameIndex ) {
                event.index--;
            }

            SystemPage.CCM__onCategoryButtonPress(event);
        }
    }
}
