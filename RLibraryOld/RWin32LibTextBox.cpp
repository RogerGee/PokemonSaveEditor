#include "RWin32LibMaster.h"
using namespace rtypes;
using namespace rwin32;

textBoxBase::textBoxBase(bool BufferLines)
{
    // this buffer is used for reading lines
    // use 0xffff number of bytes for each textbox, to ensure we get all the text 
    // (add 1 to 0xffff to ensure that there is valid space for a null terminator,
    //      in case the entire buffer is used)
    _bufferLines = BufferLines;
    if (_bufferLines)
        _lineBuf = str(0xffff+1);
    _multilineState = false;
    _delegateTextChng = 0;
    EvntTextChanged = 0;
    EvntTextUpdated = 0;
    EvntTextHScroll = 0;
    EvntTextVScroll = 0;
}
void textBoxBase::OnTextChanged(const TextBoxEventData&)
{
    // do nothing
}
void textBoxBase::OnTextUpdated(const TextBoxEventData&)
{
    // do nothing
}
void textBoxBase::OnTextHScrolled(const TextBoxEventData&)
{
    // do nothing
}
void textBoxBase::OnTextVScrolled(const TextBoxEventData&)
{
    // do nothing
}
bool textBoxBase::CreateTextBox(const Window* ParentWin)
{
    bool success;
    success = Create(-1,-1,defaults::DEFAULT_TBOX_WIDTH,defaults::DEFAULT_TBOX_HEIGHT,ParentWin);
    if (success)
    {
        // provide a textbox with a border as the default
        SetAttribute(Border);
        //determine if we are a single-line or multiline edit (we could be rich edit as well)
        _multilineState = _containsStyle(GetWindowLong(_hWnd,GWL_STYLE),ES_MULTILINE);
        if (_multilineState)
        {
            /*
                Set a 4 GB text limit. I would like to say
                    SetTextLimit(-1), which works for plain text edits as a means
                        to say "no text limit", but it doesn't work for rich edits.
                    This value is not practical, but it excludes the possibility that
                the user will be typing/pasting and will run out of room!
            */
            SetTextLimit(268435456);
        }
    }
    return success;
}
textBoxBase& textBoxBase::operator +=(const str& Text)
{
    AppendText(Text);
    return *this;
}
textBoxBase& textBoxBase::operator +=(const textBoxBase& obj)
{
    AppendText(obj.GetText());
    return *this;
}
void textBoxBase::AppendText(const str& Text)
{
    int textLength = GetTextLength();
    SetSelection(textLength,textLength);
    // append text to end
    SetSelectionText(Text);
}
void textBoxBase::InsertText(const str& Text,int At)
{
    if (At>=0)
        SetSelection(At,At);
    SetSelectionText(Text);
}
void textBoxBase::Delete()
{
    // delete the character at the current sel
    //      like pressing the DEL key
    dword textLength = dword(GetTextLength());
    dword selStart = GetSelectionStart();
    if (selStart<textLength)
    {// there is a char at the sel index to delete
        // (make the sel length 1)
        SetSelection(selStart,selStart+1);
        DeleteSelectedText();
    }
}
str textBoxBase::ParseElement(const str& Delimiters,int selStart) const
{
    const char* lineText; int lineLength;
    str s/* a buffer for single-line edit text */, elem;
    int beg, end;
    bool condition;
    int curLineStart = GetCurrentLineCharacterIndex();
    if (_bufferLines)
        lineText = GetCurrentLineText(lineLength);
    else
    {// single line edits
        s = GetText();
        lineText = s.c_str();
        lineLength = (int) s.size();
    }
    // get sel start, if needed
    if (selStart<0) selStart = (int) GetSelectionStart();
    selStart -= curLineStart; // make selection start relative to current line start index
    // calc. beg and end according to the delimiters
    beg = selStart;
    end = selStart-1; // need to test selStart; first loop iteration will increment it back up to selStart
    // calc. beg
    do
    {
        beg--;
        condition = beg>=0; // range check
        if (condition)
        {
            for (dword i = 0;i<Delimiters.size();i++)
                if (lineText[beg]==Delimiters[i])
                {
                    condition = false;
                    break;
                }
        }
    } while (condition);
    // calc. end
    do
    {
        end++;
        condition = end<lineLength; // range check
        if (condition)
        {
            for (dword i = 0;i<Delimiters.size();i++)
                if (lineText[end]==Delimiters[i])
                {
                    condition = false;
                    break;
                }
        }
    } while (condition);
    // adjust beg and end to move past delimiters/text limits
    beg++;
    end--;
    // (note - if end<beg, no element was found)
    // get the element bounded by beg and end (inclusively)
    for (;beg<=end;beg++)
        elem.push_back(lineText[beg]);
    return elem;
}
ParserElement textBoxBase::ParseElementEx(const str& Delimiters,int selStart) const
{
    const char* lineText; int lineLength;
    ParserElement elem;
    str s/* a buffer for single-line edit text */;
    bool condition;
    int curLineStart = GetCurrentLineCharacterIndex();
    if (_bufferLines)
        lineText = GetCurrentLineText(lineLength);
    else
    {// single line edits
        s = GetText();
        lineText = s.c_str();
        lineLength = (int) s.size();
    }
    // get sel start, if needed
    if (selStart<0) selStart = (int) GetSelectionStart();
    selStart -= curLineStart; // make selection start relative to current line start index
    // calc. elem.indexBegin and elem.indexEnd according to the delimiters
    elem.indexBegin = selStart;
    elem.indexEnd = selStart-1; // need to test selStart; first loop iteration will increment it back up to selStart
    // calc. elem.indexBegin
    do
    {
        elem.indexBegin--;
        condition = elem.indexBegin>=0; // range check
        if (condition)
        {
            for (dword i = 0;i<Delimiters.size();i++)
                if (lineText[elem.indexBegin]==Delimiters[i])
                {
                    condition = false;
                    break;
                }
        }
    } while (condition);
    // calc. elem.indexEnd
    do
    {
        elem.indexEnd++;
        condition = elem.indexEnd<lineLength; // range check
        if (condition)
        {
            for (dword i = 0;i<Delimiters.size();i++)
                if (lineText[elem.indexEnd]==Delimiters[i])
                {
                    condition = false;
                    break;
                }
        }
    } while (condition);
    // adjust elem.indexBegin and elem.indexEnd to move past delimiters/text limits
    elem.indexBegin++;
    elem.indexEnd--;
    // (note - if elem.indexEnd<elem.indexEnd, no element was found)
    // get and set the element bounded by elem.indexBegin and elem.indexEnd (inclusively)
    for (int i = elem.indexBegin;i<=elem.indexEnd;i++)
        elem.element.push_back(lineText[i]);
    // set element bounds relative to beginning of text
    elem.indexBegin += curLineStart;
    elem.indexEnd += curLineStart;
    return elem;
}
str textBoxBase::ParseElements(container<str>& outElements,const str& Delimiters) const
{
    str text = GetText()/* a buffer for control text */, elem /* the current element being parsed */,
        r /* the element at the current position (if any) */;
    int beg = 0, end = -1, selStart = GetSelectionStart();
    bool condition;
    // parse all items in the text based on available delimiters
    while (beg<(int)text.size())
    {
        // calc. beg
        do
        {
            beg--;
            condition = beg>=0; // range check
            if (condition)
            {
                for (dword i = 0;i<Delimiters.size();i++)
                    if (text[beg]==Delimiters[i])
                    {
                        condition = false;
                        break;
                    }
            }
        } while (condition);
        // calc. end
        do
        {
            end++;
            condition = end<(int)text.size();
            if (condition)
            {
                for (dword i = 0;i<Delimiters.size();i++)
                    if (text[end]==Delimiters[i])
                    {
                        condition = false;
                        break;
                    }
            }
        } while (condition);
        // move indeces past delimiters/text limits
        beg++;
        end--;
        // get elem
        for (dword i = beg;(int)i<=end;i++)
            elem.push_back(text[i]);
        if (elem.size())
        {
            outElements.push_back(elem);
            // check to see if elem encloses selStart
            if (selStart>=beg && selStart<=end)
                r = elem;
            elem.clear(); // clear for another read
        }
        if (end<beg)
        {
            /*
                The element wasn't found. In this case, end==beg-1, because the selection start refered to
                a delimiter. Seek beg by 1 to move on to other potential elements.
            */
            beg++;
            end = beg-1; // end needs to be 1 less than start on its first loop iteration
        }
        else
        {
            /*
                The element was found. In this case, end+1 is a logical limit to the element; move past the
                limit to other potential starts (seek to end+2, move end back 1 for loop).
            */
            end++;
            beg = end+1;
        }
    }
    return r;
}
ParserElement textBoxBase::ParseElementsEx(container<ParserElement>& outElements,const str& Delimiters) const
{
    str text = GetText()/* a buffer for control text */;
    ParserElement   elem /* the current element being parsed */,
                    r /* the element at the current position (if any) */;
    int beg = 0, end = -1, selStart = GetSelectionStart();
    bool condition;
    // parse all items in the text based on available delimiters
    while (beg<(int)text.size())
    {
        // calc. beg
        do
        {
            beg--;
            condition = beg>=0; // range check
            if (condition)
            {
                for (dword i = 0;i<Delimiters.size();i++)
                    if (text[beg]==Delimiters[i])
                    {
                        condition = false;
                        break;
                    }
            }
        } while (condition);
        // calc. end
        do
        {
            end++;
            condition = end<(int)text.size();
            if (condition)
            {
                for (dword i = 0;i<Delimiters.size();i++)
                    if (text[end]==Delimiters[i])
                    {
                        condition = false;
                        break;
                    }
            }
        } while (condition);
        // move indeces past delimiters/text limits
        beg++;
        end--;
        // get elem
        for (dword i = beg;(int)i<=end;i++)
            elem.element.push_back(text[i]);
        if (elem.element.size())
        {
            elem.indexBegin = beg;
            elem.indexEnd = end;
            outElements.push_back(elem);
            // check to see if elem encloses selStart
            if (elem.Spans(selStart))
                r = elem;
            elem.element.clear(); // clear for another read
        }
        if (end<beg)
        {
            /*
                The element wasn't found. In this case, end==beg-1, because the selection start refered to
                a delimiter. Seek beg by 1 to move on to other potential elements.
            */
            beg++;
            end = beg-1; // end needs to be 1 less than start on its first loop iteration
        }
        else
        {
            /*
                The element was found. In this case, end+1 is a logical limit to the element; move past the
                limit to other potential starts (seek to end+2, move end back 1 for loop).
            */
            end++;
            beg = end+1;
        }
    }
    return r;
}
bool textBoxBase::ParseLineElements(int LineIndex,container<str>& outElements,const str& Delimiters) const
{
    if (!_bufferLines || LineIndex>=GetNumberOfLines())
        return false;
    const char* lineText;
    int lineLength, beg = 0, end = -1;
    bool condition;
    str elem;
    // get line of text
    lineText = GetLineText(LineIndex,lineLength);
    // parse all items in the line of text based on available delimiters
    while (beg<lineLength)
    {
        // calc. beg
        do
        {
            beg--;
            condition = beg>=0; // range check
            if (condition)
            {
                for (dword i = 0;i<Delimiters.size();i++)
                    if (lineText[beg]==Delimiters[i])
                    {
                        condition = false;
                        break;
                    }
            }
        } while (condition);
        // calc. end
        do
        {
            end++;
            condition = end<lineLength;
            if (condition)
            {
                for (dword i = 0;i<Delimiters.size();i++)
                    if (lineText[end]==Delimiters[i])
                    {
                        condition = false;
                        break;
                    }
            }
        } while (condition);
        // move indeces past delimiters/text limits
        beg++;
        end--;
        // get elem
        for (dword i = beg;(int)i<=end;i++)
            elem.push_back(lineText[i]);
        if (elem.size())
        {
            outElements.push_back(elem);
            elem.clear(); // clear for another read
        }
        if (end<beg)
        {
            /*
                The element wasn't found. In this case, end==beg-1, because the selection start refered to
                a delimiter. Seek beg by 1 to move on to other potential elements.
            */
            beg++;
            end = beg-1; // end needs to be 1 less than start on its first loop iteration
        }
        else
        {
            /*
                The element was found. In this case, end+1 is a logical limit to the element; move past the
                limit to other potential starts (seek to end+2, move end back 1 for loop).
            */
            end++;
            beg = end+1;
        }
    }
    return outElements.size()>0;
}
bool textBoxBase::ParseLineElementsEx(int LineIndex,container<ParserElement>& outElements,const str& Delimiters) const
{
    if (!_bufferLines || LineIndex>=GetNumberOfLines())
        return false;
    const char* lineText;
    int lineLength, beg = 0, end = -1;
    bool condition;
    ParserElement elem;
    // get line of text
    lineText = GetLineText(LineIndex,lineLength);
    // parse all items in the line of text based on available delimiters
    while (beg<lineLength)
    {
        // calc. beg
        do
        {
            beg--;
            condition = beg>=0; // range check
            if (condition)
            {
                for (dword i = 0;i<Delimiters.size();i++)
                    if (lineText[beg]==Delimiters[i])
                    {
                        condition = false;
                        break;
                    }
            }
        } while (condition);
        // calc. end
        do
        {
            end++;
            condition = end<lineLength;
            if (condition)
            {
                for (dword i = 0;i<Delimiters.size();i++)
                    if (lineText[end]==Delimiters[i])
                    {
                        condition = false;
                        break;
                    }
            }
        } while (condition);
        // move indeces past delimiters/text limits
        beg++;
        end--;
        // get elem
        for (dword i = beg;(int)i<=end;i++)
            elem.element.push_back(lineText[i]);
        if (elem.element.size())
        {
            elem.indexBegin = beg;
            elem.indexEnd = end;
            outElements.push_back(elem);
            elem.element.clear(); // clear for another read
        }
        if (end<beg)
        {
            /*
                The element wasn't found. In this case, end==beg-1, because the selection start refered to
                a delimiter. Seek beg by 1 to move on to other potential elements.
            */
            beg++;
            end = beg-1; // end needs to be 1 less than start on its first loop iteration
        }
        else
        {
            /*
                The element was found. In this case, end+1 is a logical limit to the element; move past the
                limit to other potential starts (seek to end+2, move end back 1 for loop).
            */
            end++;
            beg = end+1;
        }
    }
    return outElements.size()>0;
}
bool textBoxBase::SetAttribute(TextBoxAttribute Attrib,bool Remove)
{
    dword style = GetWindowLong(_hWnd,GWL_STYLE);
    dword attrib = NullAttrib;
    //find correct attribs to modify
    switch (Attrib)
    {
    case NumericInputOnly:
        attrib = ES_NUMBER;
        break;
    case LowerCaseOnly:
        attrib = ES_LOWERCASE;
        break;
    case UpperCaseOnly:
        attrib = ES_UPPERCASE;
        break;
    case Password:
        {
            // have to send the control a message to change this
            bool success = SendMessage(_hWnd,EM_SETPASSWORDCHAR,(Remove ? NULL : (WPARAM) '*'),NULL)!=NULL;
            if (success)
                // redraw the textbox
                InvalidateRect(_hWnd,NULL,TRUE);
            return success;
        }
    case ReadOnly:
        // have to send the control a message to change this
        return SendMessage(_hWnd,EM_SETREADONLY,(Remove ? FALSE : TRUE),NULL)!=NULL;
    case SendReturnChar:
        attrib = ES_WANTRETURN;
        break;
    default:
        return false; // unrecognized attribute
    }
    // perform the add/remove operation
    if (Remove && _containsStyle(style,attrib))
        // bits are on, turn them off
        style ^= attrib;
    else if (!Remove && !_containsStyle(style,attrib))
        // bits are off, turn them on
        style |= attrib;
    else
        // the bits are already properly configured
        return false;
    // reset window style
    SetWindowLong(_hWnd,GWL_STYLE,style);
    // make the changes happen
    if (!SetWindowPos(_hWnd,NULL,NULL,NULL,NULL,NULL,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED))
        return false;
    return true;
}
void textBoxBase::SelectAll() const
{
    // EM_SETSEL implementation specific
    SetSelection(0,-1);
}
void textBoxBase::DeselectSelection() const
{
    // EM_SETSEL implementation specific
    SetSelection(-1,NULL);
}
bool textBoxBase::GetIfAttributeSet(TextBoxAttribute Attrib) const
{//unimplemented
    return false;
}
str textBoxBase::GetSelectedText() const
{
    str text = GetText();
    dword length, start, end;
    // calc. selection info
    length = GetSelectionInformation(start,end);
    // create a buffer for the selected text
    str buf(length+1); // this puts a null terminator at the end
    // fill buffer
    for (dword i = 0;start<=end;start++,i++)
        buf[i] = text[start];
    return buf;
}
dword textBoxBase::GetSelectionInformation(dword &Start,dword &End) const
{
    //get selection information from the edit control
    SendMessage(_hWnd,EM_GETSEL,(WPARAM) &Start,(LPARAM) &End);
    //make 'End' refer to the actual end character (make it inclusive)
    --End;
    //calculate and return selection size
    return End-Start+1;
}
dword textBoxBase::GetSelectionLength() const
{
    dword start, end;
    //get the information from the edit control
    SendMessage(_hWnd,EM_GETSEL,(WPARAM) &start,(LPARAM) &end);
    //calculate and return selection size
    return end-start;
}
dword textBoxBase::GetSelectionStart() const
{
    dword start, end;
    //get the information from the edit control
    SendMessage(_hWnd,EM_GETSEL,(WPARAM) &start,(LPARAM) &end);
    //return only the start
    return start;
}
dword textBoxBase::GetSelectionEnd() const
{
    dword start, end;
    //get the information from the edit control
    SendMessage(_hWnd,EM_GETSEL,(WPARAM) &start,(LPARAM) &end);
    //return only the end 
    return end;
}
dword textBoxBase::GetSelectionEndInclusive() const
{
    dword start, end;
    //get the information from the edit control
    SendMessage(_hWnd,EM_GETSEL,(WPARAM) &start,(LPARAM) &end);
    //return only the end 
    return (end>0 ? end-1 : end); // subtract 1 from end to get inclusive index
}
const char* textBoxBase::GetLineText(int LineIndex,int& outLineLength) const
{
    char* ourBuf = &_lineBuf[0];
    if (!_bufferLines)
    {
        // this text box doesn't buffer lines, so it can't do a get line operation
        return ourBuf; // ourBuf is at least a null string
    }
    // lineBuf contains 0xffff bytes to read a line from
    ourBuf[0] =(char) 0xff; // put the buffer size in the first word
    ourBuf[1] =(char) 0xff;
    // copy data from OS memory to lineBuf; get total number of chars copied
    // this is an ANSI operation, so the number of chars is the number of bytes
    outLineLength = SendMessage(_hWnd,EM_GETLINE,LineIndex,(LPARAM) ourBuf);
    // this copies a CRLF sequence into the buffer - strip it out
    outLineLength -= 2; // back up the count passed the two chars
    // add a null terminator to set string length - length will be zero if bad index
    _lineBuf.resize(outLineLength); // ourBuf points here; this adds a terminator to the right spot
    return ourBuf;
}
int textBoxBase::GetFirstVisibleLineIndex() const
{
    /*
        The implementation for EM_GETFIRSTVISIBLELINE will return
        a zero-based index of the first visible line IF the edit
        is single-line. For the implementation of this function, I'll
        stay true to the function name's meaning: that a line index will be 
        returned. Single-line's will return 0, always.
            */
    if (!_multilineState)
        return 0; // only 1 line
    return (int) SendMessage(_hWnd,EM_GETFIRSTVISIBLELINE,NULL,NULL);
}
int textBoxBase::ScrollText(ScrollType ByType) const
{
    UINT_PTR scrollParam;
    dword r;
    switch (ByType)
    {
    case ScrollLineUp:
        scrollParam = SB_LINEUP;
        break;
    case ScrollPageDown:
        scrollParam = SB_PAGEDOWN;
        break;
    case ScrollPageUp:
        scrollParam = SB_PAGEUP;
        break;
    case ScrollLineDown:
    default:
        scrollParam = SB_LINEDOWN;
        break;
    }
    r = SendMessage(_hWnd,EM_SCROLL,scrollParam,NULL);
    if (r>>16) // high-word is TRUE
        return (int) (r&0xffff); // return low-word
    return -1; // failure
}
void textBoxBase::_raiseSubEvents(EventData& data)
{
    // data must be a TextBoxEventData, due to the context
    TextBoxEventData& tboxData = static_cast<TextBoxEventData&> (data);
    // parse events
    switch (tboxData.MyEventType())
    {
    case TextBoxEventData::TextChanged:
        {
            Window* par = const_cast<Window*> ( GetParentWin() );
            if (par)
            {// the edit has a parent
                if (_delegateTextChng)
                    (par->*_delegateTextChng)(this);
            }
            if (EvntTextChanged)
            {
                data.SetFunctCall(EvntTextChanged);
                data.RaiseEvent();
            }
            OnTextChanged(tboxData);
        }
        break;
    case TextBoxEventData::TextUpdated:
        if (EvntTextUpdated)
        {
            data.SetFunctCall(EvntTextUpdated);
            data.RaiseEvent();
        }
        OnTextUpdated(tboxData);
        break;
    case TextBoxEventData::HScrolled:
        if (EvntTextHScroll)
        {
            data.SetFunctCall(EvntTextHScroll);
            data.RaiseEvent();
        }
        OnTextHScrolled(tboxData);
        break;
    case TextBoxEventData::VScrolled:
        if (EvntTextVScroll)
        {
            data.SetFunctCall(EvntTextVScroll);
            data.RaiseEvent();
        }
        OnTextVScrolled(tboxData);
        break;
    default:
        _raiseSpecificSubEvents(tboxData);
        break;
    }
}

TextBox::TextBox()
    : textBoxBase(false) // single-lines don't need line buffering
{
}
bool TextBox::SetBannerCue(wstr BannerText,bool ShowWhenFocused)
{
    BannerText.truncate(defaults::BANNER_BUFFER_SIZE);
    return SendMessage(_hWnd,EM_SETCUEBANNER,ShowWhenFocused,(LPARAM) BannerText.c_str())==TRUE;
}
bool TextBox::SetTextBoxSize(int Width)
{
    return SetSize(Width,/*Get from font*/22);
}
wstr TextBox::GetBannerCue() const
{
    wstr buffer(defaults::BANNER_BUFFER_SIZE+1);
    if (SendMessage(_hWnd,EM_GETCUEBANNER,(WPARAM) &buffer[0],defaults::BANNER_BUFFER_SIZE+1)!=TRUE)
        return wstr();
    // set the string size for accurate reporting
    dword size = 0;
    while (size<defaults::BANNER_BUFFER_SIZE && buffer[size])
        size++;
    buffer.truncate(size);
    return buffer;
}
const char* TextBox::_getWinClass() const
{
    return WC_EDIT;
}
dword TextBox::_getWinStyle() const
{
    return WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_AUTOVSCROLL;
}
Point TextBox::_getDefaultWinLocation(int Width,int Height) const
{
    return Point();
}
void TextBox::_raiseSpecificSubEvents(TextBoxEventData& data)
{
}

MultilineTextBox::MultilineTextBox()
    : textBoxBase(true) // allocate extra data to help with line reads
{
}
const char* MultilineTextBox::_getWinClass() const
{
    return WC_EDIT;
}
dword MultilineTextBox::_getWinStyle() const
{
    return WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE;
}
Point MultilineTextBox::_getDefaultWinLocation(int Width,int Height) const
{
    return Point();
}
void MultilineTextBox::_raiseSpecificSubEvents(TextBoxEventData& data)
{
}

RichCharAttrib::RichCharAttrib()
{
    bold = false;
    italic = false;
    strikeout = false;
    underline = false;
    protectedCharacter = false;
    height = 0;
    offset = 0;
    // the default is that all specified attribs are changed,
    //      unless they have a null value (e.g. for height)
    _changeColor = true;
    _changeFontFamily = true;
    _changeHeight = true;
    _changeOffset = true;
    _changeProtected = true;
    _changeTextStyle = true;
}
void RichCharAttrib::SetChanges(bool ChangeHeight,
                                bool ChangeOffset,
                                bool ChangeColor,
                                bool ChangeFontFamily,
                                bool ChangeTextStyle,
                                bool ChangeProtected)
{
    _changeColor = ChangeColor;
    _changeFontFamily = ChangeFontFamily;
    _changeHeight = ChangeHeight;
    _changeOffset = ChangeOffset;
    _changeProtected = ChangeProtected;
    _changeTextStyle = ChangeTextStyle; // all text style flags either are applied or aren't
}
void RichCharAttrib::_fillCharFormatStruct(CHARFORMAT* strct) const
{
    // set struct size
    strct->cbSize = sizeof(CHARFORMAT);
    // set mask flags so the system will no which ones to ignore
    strct->dwMask = NULL;
    if (_changeColor)
        strct->dwMask |= CFM_COLOR;
    if (_changeFontFamily && fontFamily.size()>0)
        strct->dwMask |= CFM_FACE;
    if (_changeHeight && height>0) // change only if height is >0
        strct->dwMask |= CFM_SIZE;
    if (_changeOffset)
        strct->dwMask |= CFM_OFFSET;
    if (_changeProtected)
        strct->dwMask |= CFM_PROTECTED;
    if (_changeTextStyle)
    {
        strct->dwMask |= CFM_BOLD;
        strct->dwMask |= CFM_ITALIC;
        strct->dwMask |= CFM_UNDERLINE;
        strct->dwMask |= CFM_STRIKEOUT;
    }
    // set effect flags to determine how the text is changed
    strct->dwEffects = NULL;
    if (italic)
        strct->dwEffects |= CFE_ITALIC;
    if (bold)
        strct->dwEffects |= CFE_BOLD;
    if (underline)
        strct->dwEffects |= CFE_UNDERLINE;
    if (strikeout)
        strct->dwEffects |= CFE_STRIKEOUT;
    if (protectedCharacter)
        strct->dwEffects |= CFE_PROTECTED;
    if (textColor.IsNull())
        strct->dwEffects |= CFE_AUTOCOLOR; // this causes the crTextColor member to be ignored
    else
        strct->crTextColor = textColor;
    strct->yHeight = height;
    strct->yOffset = offset;
    //      copy font family name into buffer in CHARFORMAT struct; limit 32 chars (including terminating character)
    dword i;
    for (i = 0;i<LF_FACESIZE-1 && i<fontFamily.size();i++)
        strct->szFaceName[i] = fontFamily[i];
    strct->szFaceName[i] = '\0'; // null-terminate the string
}
void RichCharAttrib::_setFromCharFormatStruct(const CHARFORMAT* strct)
{
    _changeColor = (strct->dwMask&CFM_COLOR) == CFM_COLOR;
    _changeFontFamily = (strct->dwMask&CFM_FACE) == CFM_FACE;
    _changeHeight = (strct->dwMask&CFM_SIZE) == CFM_SIZE;
    _changeOffset = (strct->dwMask&CFM_OFFSET) == CFM_OFFSET;
    _changeProtected = (strct->dwMask&CFM_PROTECTED) == CFM_PROTECTED;
    _changeTextStyle = false;
    if ((strct->dwMask&CFM_BOLD) == CFM_BOLD)
    {
        bold = (strct->dwEffects&CFE_BOLD) == CFE_BOLD;
        _changeTextStyle = true;
    }
    else
        bold = false;
    if ((strct->dwMask&CFM_ITALIC) == CFM_ITALIC)
    {
        italic = (strct->dwEffects&CFE_ITALIC) == CFE_ITALIC;
        _changeTextStyle = true;
    }
    else
        italic = false;
    if ((strct->dwMask&CFM_UNDERLINE) == CFM_UNDERLINE)
    {
        underline = (strct->dwEffects&CFE_UNDERLINE) == CFE_UNDERLINE;
        _changeTextStyle = true;
    }
    else
        underline = false;
    if ((strct->dwMask&CFM_STRIKEOUT) == CFM_STRIKEOUT)
    {
        strikeout = (strct->dwEffects&CFE_STRIKEOUT) == CFE_STRIKEOUT;
        _changeTextStyle = true;
    }
    else
        strikeout = false;
    // color
    if ((strct->dwEffects&CFE_AUTOCOLOR) == CFE_AUTOCOLOR)
        textColor = GetSysColor(COLOR_WINDOWTEXT); // assign default system color
    else
        textColor = strct->crTextColor;
    // 
    protectedCharacter = (strct->dwEffects&CFE_PROTECTED) == CFE_PROTECTED;
    if (_changeHeight)
        height = strct->yHeight;
    else
        height = 0;
    if (_changeOffset)
        offset = strct->yOffset;
    else
        offset = 0;
    // get font family string
    fontFamily.clear();
    dword i = 0;
    while (strct->szFaceName[i])
        fontFamily.push_back(strct->szFaceName[i++]);
}

DroppedFileInfo::DroppedFileInfo()
{
    characterPosition = 0;
}
void DroppedFileInfo::_setFromDropFilesStruct(const ENDROPFILES* strct)
{
    // get the character position (at least there's one easy thing to get)
    characterPosition = strct->cp;
    // get the file names;
    dword cnt = 0;
    //      first, get the number of files dropped
    cnt = DragQueryFile((HDROP) strct->hDrop,0xffffffff,NULL,NULL); // pass -1 to DrawQueryFile to get number of files
    //      resize fixed container to number of files
    fileNames.resize(cnt);
    //      get each file name
    for (dword i = 0;i<cnt;i++)
    {
        str temp(MAX_PATH); // allocate a max file buffer to temporarily store the file name
        if ( !DragQueryFile((HDROP) strct->hDrop,i,&temp[0],MAX_PATH) )
            fileNames[i] = "<error>";
        else
            fileNames[i] = temp; // this will trim the allocation for fileNames[i] in case all the data in temp wasn't used
    }
    // get the mouse position
    POINT pntTemp;
    if ( DragQueryPoint((HDROP) strct->hDrop,&pntTemp) )
    {
        mousePosition.x = pntTemp.x;
        mousePosition.y = pntTemp.y;
    }
    else
        mousePosition = Point(); // in case there's something else there
    // release drag drop data
    DragFinish((HDROP) strct->hDrop);
}

RichTextBox::RichTextBox()
    : textBoxBase(true) // buffer data because control is multlined
{
    _delegateSelChng = 0;
    _delegateFileDrop = 0;
    EvntSelectionChanged = 0;
    EvntFileDropped = 0;
}
bool RichTextBox::FormatAllText(const RichCharAttrib& Formatting)
{
    CHARFORMAT charFormat; CHARFORMAT *pFormat = &charFormat;
    // get the CHARFORMAT struct from the Formatting object
    Formatting._fillCharFormatStruct(pFormat);
    // send the message to change the formatting for the all of the text in the edit
    return SendMessage(_hWnd,EM_SETCHARFORMAT,SCF_ALL,(LPARAM)pFormat)!=0;
}
bool RichTextBox::FormatSelectedText(const RichCharAttrib& Formatting)
{
    CHARFORMAT charFormat; CHARFORMAT *pFormat = &charFormat;
    // get the CHARFORMAT struct from the Formatting object
    Formatting._fillCharFormatStruct(pFormat);
    // send the message to change the formatting for the selected text in the edit
    return SendMessage(_hWnd,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM)pFormat)!=0;
}
RichCharAttrib RichTextBox::GetTextFormatting() const
{
    RichCharAttrib formatting;
    CHARFORMAT charFormat;
    // set size so redit proc. knows what kind of structure it is
    charFormat.cbSize = sizeof(CHARFORMAT);
    // get formatting attribs to CHARFORMAT structure
    SendMessage(_hWnd,EM_GETCHARFORMAT,SCF_DEFAULT,(LPARAM)&charFormat);
    // put CHARFORMAT data into a RichCharAttrib object
    formatting._setFromCharFormatStruct(&charFormat);
    return formatting;
}
RichCharAttrib RichTextBox::GetSelectedTextFormatting() const
{
    RichCharAttrib formatting;
    CHARFORMAT charFormat;
    // set size so redit proc. knows what kind of structure it is
    charFormat.cbSize = sizeof(CHARFORMAT);
    // get formatting attribs to CHARFORMAT structure
    SendMessage(_hWnd,EM_GETCHARFORMAT,SCF_SELECTION,(LPARAM)&charFormat);
    // put CHARFORMAT data into a RichCharAttrib object
    formatting._setFromCharFormatStruct(&charFormat);
    return formatting;
}
str RichTextBox::GetSelectedText() const
{
    str buf( GetSelectionLength()+1 ); // add 1 for null terminator
    dword charsCopied = SendMessage(_hWnd,EM_GETSELTEXT,NULL,(LPARAM) &buf[0]);
    buf.truncate(charsCopied); // just in case
    return buf;
}
const char* RichTextBox::_getWinClass() const
{
    return "RICHEDIT";
}
dword RichTextBox::_getWinStyle() const
{
    return WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_HSCROLL | WS_VSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_LEFT;
}
Point RichTextBox::_getDefaultWinLocation(int Width,int Height) const
{
    return Point();
}
void RichTextBox::_raiseSpecificSubEvents(TextBoxEventData& data)
{
    Window* par = const_cast<Window*> ( GetParentWin() );
    switch (data.MyEventType())
    {
        // rich edits handle SelectionChanged and FileDropped events
    case TextBoxEventData::SelectionChanged:
        // invoke the delegate
        if (par && _delegateSelChng)
            (par->*_delegateSelChng)(this);
        // raise other event handlers
        if (EvntSelectionChanged)
        {
            data.SetFunctCall(EvntSelectionChanged);
            data.RaiseEvent();
        }
        OnSelectionChanged(data);
        break;
    case TextBoxEventData::FileDropped:
        {
            // get dropped file information
            DroppedFileInfo info;
            if (data.tag)
                info._setFromDropFilesStruct( reinterpret_cast<const ENDROPFILES*> (data.tag) ); // this releases drag drop memory
            // invoke the delegate
            if (par && _delegateFileDrop)
                (par->*_delegateFileDrop)(info);
            // raise other event handlers
            if (EvntFileDropped)
            {
                data.SetFunctCall(EvntFileDropped);
                data.RaiseEvent();
            }
            OnFileDropped(info);
        }
        break;
    }
}
void RichTextBox::OnFileDropped(const DroppedFileInfo&)
{
    // do nothing
}
void RichTextBox::OnSelectionChanged(const TextBoxEventData&)
{
    // do nothing
}
void RichTextBox::OnCreate(const EventData& data)
{
    /*  Perform startup tasks for rich edit controls - any user overriding the OnCreate method will have to implement their own specific operation;
        this could conflict with normal rich edit operation.
    */
    // Set event masks for a rich edit control so that it will send notification messages
    // to its parents's window procedure
    dword eventMasks;
    eventMasks = ENM_CHANGE;
    eventMasks |= ENM_DROPFILES;
    eventMasks |= ENM_KEYEVENTS;
    eventMasks |= ENM_MOUSEEVENTS;
    eventMasks |= ENM_SCROLL;
    eventMasks |= ENM_SELCHANGE;
    // set event masks
    SendMessage(_hWnd,EM_SETEVENTMASK,NULL,eventMasks);
    // Enable shell commands (drag 'n drop) to be sent to the control
    DragAcceptFiles(_hWnd,TRUE);
}