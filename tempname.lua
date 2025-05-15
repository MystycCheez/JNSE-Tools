local uiAvailable <const> = app.isUIAvailable

local fileTypes <const> = { "pbm", "pgm", "ppm" }
local writeModes <const> = { "ASCII", "BINARY" }
local colorModes <const> = { "RGB", "GRAY", "INDEXED" }
local units <const> = { "BITS", "INTEGERS" }

---@param exportFilepath string
---@param activeSprite Sprite
---@param frIdcs integer[]
---@param writeMode "ASCII"|"BINARY"
---@param channelMax integer
---@param pivot integer
---@param scale integer
---@param usePixelAspect boolean
local function writeFile(
    exportFilepath,
    activeSprite,
    frIdcs,
    writeMode,
    channelMax,
    pivot,
    scale,
    usePixelAspect)
    -- Check for invalid file extension.
    local fileSysTools <const> = app.fs
    local fileExt <const> = fileSysTools.fileExtension(exportFilepath)
    local filePathAndTitle <const> = string.gsub(
        fileSysTools.filePathAndTitle(exportFilepath), "\\", "\\\\")

    local fileExtLower <const> = string.lower(fileExt)
    local extIsPbm <const> = fileExtLower == "pbm"

    if not (extIsPbm or extIsPgm or extIsPpm) then
        if uiAvailable then
            app.alert {
                title = "Error",
                text = "File extension must be pbm, pgm or ppm."
            }
        else
            print("Error: File extension must be pbm, pgm or ppm.")
        end
        return
    end

    -- For the pbm file extension, black is associated with 1 or on,
    -- and white is associated with 0 or off. This is the opposite of
    -- image conventions.
    local offTok <const> = "1"
    local onTok <const> = "0"

    -- Unpack sprite data.
    local palettes <const> = activeSprite.palettes
    local lenPalettes <const> = #palettes
    local spriteSpec <const> = activeSprite.spec
    local wSprite <const> = spriteSpec.width
    local hSprite <const> = spriteSpec.height
    local colorMode <const> = spriteSpec.colorMode

    -- Process color mode.
    local cmIsRgb <const> = colorMode == ColorMode.RGB
    local cmIsGry <const> = colorMode == ColorMode.GRAY
    local cmIsIdx <const> = colorMode == ColorMode.INDEXED

    -- Process mode.
    local fmtIsBinary <const> = writeMode == "BINARY"
    local fmtIsAscii <const> = writeMode == "ASCII"

    -- Process channel max.
    local chnlMaxVerif <const> = math.min(math.max(channelMax, 1), 255)
    local toChnlMax <const> = chnlMaxVerif / 255.0
    local frmtrStr = "%d"
    if fmtIsAscii then
        if chnlMaxVerif < 10 then
            frmtrStr = "%01d"
        elseif chnlMaxVerif < 100 then
            frmtrStr = "%02d"
        elseif chnlMaxVerif < 1000 then
            frmtrStr = "%03d"
        end
    end

    -- Process scale.
    local wScale = scale
    local hScale = scale
    if usePixelAspect then
        -- Pixel ratio sizes are not validated by Aseprite.
        local pxRatio <const> = activeSprite.pixelRatio
        local pxw <const> = math.max(1, math.abs(pxRatio.width))
        local pxh <const> = math.max(1, math.abs(pxRatio.height))
        wScale = wScale * pxw
        hScale = hScale * pxh
    end
    local useResize <const> = wScale ~= 1 or hScale ~= 1
    local wSpriteScld <const> = math.min(wSprite * wScale, 65535)
    local hSpriteScld <const> = math.min(hSprite * hScale, 65535)
    local imageSizeStr <const> = string.format(
        "%d %d",
        wSpriteScld, hSpriteScld)

    -- Supplied to image pixel method when looping
    -- by pixel row.
    local rowRect <const> = Rectangle(0, 0, wSpriteScld, 1)

    -- Cache global methods to local.
    local floor <const> = math.floor
    local ceil <const> = math.ceil
    local strfmt <const> = string.format
    local strpack <const> = string.pack
    local strsub <const> = string.sub
    local strgsub <const> = string.gsub
    local tconcat <const> = table.concat
    local tinsert <const> = table.insert

    -- For the binary format, the code supplied to io.open is different.
    -- The separators for columns and rows are not needed.
    local writerType = "w"
    local colSep = " "
    local rowSep = "\n"
    if fmtIsBinary then
        writerType = "wb"
        colSep = ""
        rowSep = ""
    end

    -- The appropriate string for a pixel differs based on (1.) the
    -- extension, (2.) the sprite color mode, (3.) whether ASCII or binary
    -- is being written. Binary .pbm files are a special case because bits
    -- are packed into byte-sized ASCII chars.
    local headerStr = ""
    local chnlMaxStr = ""
    local isBinPbm = false
    local writePixel = nil

    if extIsPpm then
        -- File extension supports RGB.
        headerStr = "P3"
        chnlMaxStr = strfmt("%d", chnlMaxVerif)
        local rgbFrmtrStr = strfmt(
            "%s %s %s",
            frmtrStr, frmtrStr, frmtrStr)
        if fmtIsBinary then
            headerStr = "P6"
            rgbFrmtrStr = "%s%s%s"
        end

        if cmIsIdx then
            if fmtIsBinary then
                writePixel = function(h, p)
                    local c <const> = p:getColor(h)
                    return strpack("B B B",
                        floor(c.red * toChnlMax + 0.5),
                        floor(c.green * toChnlMax + 0.5),
                        floor(c.blue * toChnlMax + 0.5))
                end
            else
                writePixel = function(h, p)
                    local c <const> = p:getColor(h)
                    return strfmt(rgbFrmtrStr,
                        floor(c.red * toChnlMax + 0.5),
                        floor(c.green * toChnlMax + 0.5),
                        floor(c.blue * toChnlMax + 0.5))
                end
            end
        elseif cmIsGry then
            if fmtIsBinary then
                writePixel = function(h)
                    local v <const> = floor((h & 0xff) * toChnlMax + 0.5)
                    return strpack("B B B", v, v, v)
                end
            else
                writePixel = function(h)
                    local v <const> = floor((h & 0xff) * toChnlMax + 0.5)
                    return strfmt(rgbFrmtrStr, v, v, v)
                end
            end
        else
            -- Default to RGB color mode.
            if fmtIsBinary then
                writePixel = function(h)
                    return strpack("B B B",
                        floor((h & 0xff) * toChnlMax + 0.5),
                        floor((h >> 0x08 & 0xff) * toChnlMax + 0.5),
                        floor((h >> 0x10 & 0xff) * toChnlMax + 0.5))
                end
            else
                writePixel = function(h)
                    return strfmt(rgbFrmtrStr,
                        floor((h & 0xff) * toChnlMax + 0.5),
                        floor((h >> 0x08 & 0xff) * toChnlMax + 0.5),
                        floor((h >> 0x10 & 0xff) * toChnlMax + 0.5))
                end
            end
        end
    elseif extIsPgm then
        -- File extension supports grayscale.
        -- From Wikipedia:
        -- "Conventionally PGM stores values in linear color space, but
        -- depending on the application, it can often use either sRGB or a
        -- simplified gamma representation."

        headerStr = "P2"
        chnlMaxStr = strfmt("%d", chnlMaxVerif)
        if fmtIsBinary then
            headerStr = "P5"
        end

        if cmIsIdx then
            if fmtIsBinary then
                writePixel = function(h, p)
                    local c <const> = p:getColor(h)
                    return strpack("B", floor(lum(
                        c.red, c.green, c.blue) * toChnlMax + 0.5))
                end
            else
                writePixel = function(h, p)
                    local c <const> = p:getColor(h)
                    return strfmt(frmtrStr, floor(lum(
                        c.red, c.green, c.blue) * toChnlMax + 0.5))
                end
            end
        elseif cmIsRgb then
            if fmtIsBinary then
                writePixel = function(h)
                    return strpack("B", floor(lum(
                        h & 0xff,
                        h >> 0x08 & 0xff,
                        h >> 0x10 & 0xff) * toChnlMax + 0.5))
                end
            else
                writePixel = function(h)
                    return strfmt(frmtrStr, floor(lum(
                        h & 0xff,
                        h >> 0x08 & 0xff,
                        h >> 0x10 & 0xff) * toChnlMax + 0.5))
                end
            end
        else
            -- Default to grayscale color mode.
            if fmtIsBinary then
                writePixel = function(h)
                    return strpack("B", floor((h & 0xff) * toChnlMax + 0.5))
                end
            else
                writePixel = function(h)
                    return strfmt(frmtrStr, floor((h & 0xff) * toChnlMax + 0.5))
                end
            end
        end
    else
        -- Default to extIsPbm (1 or 0).
        headerStr = "P1"
        if fmtIsBinary then
            headerStr = "P4"
            isBinPbm = true
        end

        if cmIsGry then
            writePixel = function(h)
                if (h & 0xff) < pivot then return offTok end
                return onTok
            end
        elseif cmIsRgb then
            writePixel = function(h)
                if lum(h & 0xff, h >> 0x08 & 0xff,
                        h >> 0x10 & 0xff) < pivot then
                    return offTok
                end
                return onTok
            end
        else
            writePixel = function(h, p)
                local c <const> = p:getColor(h)
                if lum(c.red, c.green, c.blue) < pivot then
                    return offTok
                end
                return onTok
            end
        end
    end

    -- Store unique pixels in a dictionary, where each pixel value is the
    -- key and its string representation is the value. (For multiple frames,
    -- initialize this dictionary outside the frames loop.)
    ---@type table<integer, string>
    local hexToStr <const> = {}

    local lenFrIdcs <const> = #frIdcs
    local h = 0
    while h < lenFrIdcs do
        h = h + 1
        local frIdx <const> = frIdcs[h]

        -- In rare cases, e.g., a sprite opened from a sequence of indexed
        -- color mode files, there may be multiple palettes in the sprite.
        local paletteIdx = frIdx
        if paletteIdx > lenPalettes then paletteIdx = 1 end
        local palette <const> = palettes[paletteIdx]

        -- Blit the sprite composite onto a new image.
        local trgImage <const> = Image(spriteSpec)
        trgImage:drawSprite(activeSprite, frIdx)
        local trgPxItr <const> = trgImage:pixels()

        -- Convert pixels to strings.
        for pixel in trgPxItr do
            local hex <const> = pixel()
            if not hexToStr[hex] then
                hexToStr[hex] = writePixel(hex, palette)
            end
        end

        -- Scale image after unique pixels have been found.
        if useResize then
            trgImage:resize(wSpriteScld, hSpriteScld)
        end

        -- Concatenate pixels into  columns, then rows.
        ---@type string[]
        local rowStrs <const> = {}
        local j = 0
        while j < hSpriteScld do
            ---@type string[]
            local colStrs <const> = {}
            rowRect.y = j
            local rowItr <const> = trgImage:pixels(rowRect)
            for rowPixel in rowItr do
                colStrs[#colStrs + 1] = hexToStr[rowPixel()]
            end

            j = j + 1
            rowStrs[j] = tconcat(colStrs, colSep)
        end

        local frFilePath = exportFilepath
        if lenFrIdcs > 1 then
            frFilePath = strfmt("%s_%03d.%s", filePathAndTitle, frIdx, fileExt)
        end

        local file <const>, err <const> = io.open(frFilePath, writerType)
        if err ~= nil then
            if file then file:close() end
            if uiAvailable then
                app.alert { title = "Error", text = err }
            else
                print(err)
            end
            return
        end

        if file == nil then
            if uiAvailable then
                app.alert {
                    title = "Error",
                    text = "File could not be opened."
                }
            else
                print(strfmt("Error: Could not open file \"%s\".", frFilePath))
            end
            return
        end

        local imgDataStr = ""
        if isBinPbm then
            -- From Wikipedia:
            -- "The P4 binary format of the same image represents each pixel
            -- with a single bit, packing 8 pixels per byte, with the first
            -- pixel as the most significant bit. Extra bits are added at the
            -- end of each row to fill a whole byte."

            ---@type string[]
            local charStrs <const> = {}
            local lenRows <const> = #rowStrs
            local k = 0
            while k < lenRows do
                k = k + 1
                local rowStr <const> = rowStrs[k]
                local lenRowStr <const> = #rowStr
                local lenRowChars <const> = ceil(lenRowStr / 8)

                local m = 0
                while m < lenRowChars do
                    local idxOrig <const> = 1 + m * 8
                    local idxDest <const> = idxOrig + 7
                    local strSeg = strsub(rowStr, idxOrig, idxDest)
                    while #strSeg < 8 do strSeg = strSeg .. offTok end
                    local numSeg <const> = tonumber(strSeg, 2)
                    charStrs[#charStrs + 1] = strpack("B", numSeg)
                    m = m + 1
                end
            end

            imgDataStr = tconcat(charStrs)
        else
            imgDataStr = tconcat(rowStrs, rowSep)
        end

        ---@type string[]
        local chunks <const> = { headerStr, imageSizeStr, imgDataStr }

        -- When an ASCII file does not have an extra blank line or white space
        -- at the end, the bottom right pixel will appear incorrect in
        -- Photopea and Irfanview.
        if fmtIsAscii then
            table.insert(chunks, "")
        end

        if not extIsPbm then
            tinsert(chunks, 3, chnlMaxStr)
        end
        file:write(tconcat(chunks, "\n"))
        file:close()

        if not uiAvailable then
            print(strfmt("Wrote file to %s .",
                strgsub(frFilePath, "\\+", "\\")))
        end
    end
end