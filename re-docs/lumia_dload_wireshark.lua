-- Nokia Lumia DLOAD Wireshark Analyzer
-- by Emma / InvoxiPlayGames
-- https://github.com/InvoxiPlayGames/lumia-dloadtool

dload_protocol = Proto("dload", "Nokia DLOAD")

msg_type_table = {
    [0xbb0bb001] = "Echo",
    [0xbb0bb002] = "Reset",
    [0xbb0bb003] = "Version Get",
    [0xbb0bb004] = "Version Reply",
    [0xbb0bb005] = "Power Off",
    [0xbb6bb601] = "BB6 Message: "
}

bb6_tlv_table = {
    [0x10001] = "Log Message",
    [0x10002] = "Data",
    [0x10003] = "Control Message",
    [0x20001] = "Image Metadata",
    [0x20002] = "File Metadata",
    [0x20003] = "Cert Metadata",
    [0x30001] = "Request",
    [0x30002] = "Response",
    -- no clue about these two
    [0x40001] = "Get Nokia Key",
    [0x40002] = "Submit Nokia Signature"
}

bb6_cmd_table = {
    [0x1] = "Reset",
    [0x3] = "Control",
    [0x4] = "Send Certificate",
    [0x6] = "Power Off",
    [0x100] = "Write Data",
    [0x101] = "Erase Data",
    [0x104] = "Write Delayed",
    [0x105] = "Flush Delayed",
    [0x106] = "Erase Delayed",
    -- unsure about these two
    [0x200] = "Nokia Command",
    [0x10000] = "Modem Backup/Restore"
}

dload_msg_type = ProtoField.uint32("dload.msg_type", "Message Type", base.HEX, msg_type_table)
dload_msg_length = ProtoField.uint32("dload.msg_length", "Message Length")

dload_echo = ProtoField.bytes("dload.echo", "Echoed Data", base.NONE)

dload_ver = ProtoField.protocol("dload.version", "DLOAD Version")
dload_ver_unk = ProtoField.bytes("dload.version.unk", "Unknown", base.NONE)
dload_ver_timestamp = ProtoField.absolute_time("dload.version.timestamp", "Build Timestamp")

dload_bb6 = ProtoField.protocol("dload.bb6", "BB6 Message Info")
dload_bb6_unk1 = ProtoField.uint32("dload.bb6.unk1")
dload_bb6_unk2 = ProtoField.uint32("dload.bb6.unk2", "Message Type")
dload_bb6_count = ProtoField.uint32("dload.bb6.tlv_count", "TLV Count")

dload_req = ProtoField.protocol("dload.req", "Request")
dload_req_cmd = ProtoField.uint32("dload.req.cmd", "Command ID", base.HEX, bb6_cmd_table)
dload_req_seq = ProtoField.uint32("dload.req.sequence", "Sequence Number")

dload_resp = ProtoField.protocol("dload.resp", "Response")
dload_resp_seq = ProtoField.uint32("dload.resp.sequence", "Sequence Number")
dload_resp_rcode = ProtoField.uint32("dload.resp.return_code", "Return Code", base.HEX)
dload_resp_unk = ProtoField.uint32("dload.resp.unk")

dload_ctrl = ProtoField.protocol("dload.ctrl", "Control Message")
dload_ctrl_id = ProtoField.uint32("dload.ctrl.id", "Control ID", base.HEX)
dload_ctrl_len = ProtoField.uint32("dload.ctrl.len", "Length")
dload_ctrl_data = ProtoField.bytes("dload.ctrl.data", "Data", base.NONE)

dload_cert = ProtoField.protocol("dload.cert", "Certificate Metadata")
dload_cert_length = ProtoField.uint32("dload.cert.length", "Length")
dload_cert_unk = ProtoField.uint32("dload.cert.unk")
dload_cert_kid = ProtoField.uint32("dload.cert.key_id", "Key ID (?)")

dload_flash = ProtoField.protocol("dload.flash", "Image Metadata")
dload_flash_length = ProtoField.uint64("dload.flash.length", "Length", base.HEX)
dload_flash_addr = ProtoField.uint64("dload.flash.address", "Address", base.HEX)
dload_flash_offset = ProtoField.uint64("dload.flash.offset", "Offset", base.HEX)
dload_flash_unk = ProtoField.uint32("dload.flash.unk")
dload_flash_kid = ProtoField.uint32("dload.flash.key_id", "Key ID (?)")

dload_data = ProtoField.bytes("dload.data", "Data", base.NONE)

dload_unparsed = ProtoField.bytes("dload.unknown", "Unparsed Data", base.NONE)

-- register the protocol fields
dload_protocol.fields = {
    dload_unparsed,

    dload_msg_type,
    dload_msg_length,

    dload_echo,

    dload_ver,
    dload_ver_unk,
    dload_ver_timestamp,

    dload_bb6,
    dload_bb6_unk1,
    dload_bb6_unk2,
    dload_bb6_count,

    dload_req,
    dload_req_cmd,
    dload_req_seq,

    dload_resp,
    dload_resp_seq,
    dload_resp_rcode,
    dload_resp_unk,

    dload_ctrl,
    dload_ctrl_id,
    dload_ctrl_len,
    dload_ctrl_data,

    dload_cert,
    dload_cert_length,
    dload_cert_unk,
    dload_cert_kid,

    dload_flash,
    dload_flash_length,
    dload_flash_addr,
    dload_flash_offset,
    dload_flash_unk,
    dload_flash_kid,

    dload_data
}

function dload_protocol.dissector(buffer, pinfo, tree)
    length = buffer:len()
    if length < 8 then return end

    local msg_type = buffer(0, 4):le_uint()
    local msg_length = buffer(4, 4):le_uint() -- the bootloader seems to largely ignore this

    -- comment if you think something is missing
    if not msg_type_table[msg_type] then return end

    local subtree = tree:add(dload_protocol, buffer())
    subtree:add_le(dload_msg_type, buffer(0, 4))
    subtree:add_le(dload_msg_length, buffer(4, 4))

    pinfo.cols.protocol = dload_protocol.name
    pinfo.cols.info = (msg_type_table[msg_type] or string.format("Unknown 0x%08x", msg_type))

    if msg_type == 0xbb6bb601 and msg_length == 0xC then
        cmdinfo_buf = buffer(8, 0xC)
        local cmdtree = subtree:add(dload_bb6, cmdinfo_buf)
        cmdtree:add_le(dload_bb6_unk1, cmdinfo_buf(0, 4))
        cmdtree:add_le(dload_bb6_unk2, cmdinfo_buf(4, 4))
        cmdtree:add_le(dload_bb6_count, cmdinfo_buf(8, 4))
        cmd_count = cmdinfo_buf(8, 4):le_uint()
        cur_pos = 0x14
        for i=1,cmd_count do
            cmd_type = buffer(cur_pos, 0x4):le_uint()
            cmd_len = buffer(cur_pos + 0x4, 0x4):le_uint()
            if cmd_len == 0 then
                cmd_len = length - cur_pos - 0x8
            end
            cmd_buf = buffer(cur_pos + 0x8, cmd_len)
            cmd_info = ""

            if cmd_type == 0x30001 and cmd_len == 0x8 then
                -- request metadata
                cmd_id = cmd_buf(4, 0x4):le_uint()
                req_tree = subtree:add(dload_req, cmd_buf)
                req_tree:add_le(dload_req_seq, cmd_buf(0, 0x4))
                req_tree:add_le(dload_req_cmd, cmd_buf(4, 0x4))
                cmd_info = " (" .. (bb6_cmd_table[cmd_id] or string.format("Unknown 0x%x", cmd_id)) .. ")"
            elseif cmd_type == 0x30002 and cmd_len == 0xC then
                -- response metadata
                resp_tree = subtree:add(dload_resp, cmd_buf)
                resp_tree:add_le(dload_resp_seq, cmd_buf(0, 0x4))
                resp_tree:add_le(dload_resp_rcode, cmd_buf(4, 0x4))
                resp_tree:add_le(dload_resp_unk, cmd_buf(8, 0x4))
            elseif cmd_type == 0x10002 then
                -- binary data
                if length + 27 >= 65535 then
                    -- Wireshark/USBpcap on Windows caps out at 0x10000 bytes
                    cmd_info = " (more than " .. cmd_len .. " bytes)"
                else
                    cmd_info = " (" .. cmd_len .. " bytes)"
                end
                subtree:add(dload_data, cmd_buf)
            elseif cmd_type == 0x20001 and cmd_len == 0x20 then
                -- flash metadata
                flash_tree = subtree:add(dload_flash, cmd_buf)
                flash_tree:add_le(dload_flash_length, cmd_buf(0x00, 8))
                flash_tree:add_le(dload_flash_addr, cmd_buf(0x08, 8))
                flash_tree:add_le(dload_flash_offset, cmd_buf(0x10, 8))
                flash_tree:add_le(dload_flash_unk, cmd_buf(0x18, 4))
                flash_tree:add_le(dload_flash_kid, cmd_buf(0x1C, 4))
            elseif cmd_type == 0x20003 and cmd_len == 0xC then
                -- certificate metadata
                cert_tree = subtree:add(dload_cert, cmd_buf)
                cert_tree:add_le(dload_cert_length, cmd_buf(0, 4))
                cert_tree:add_le(dload_cert_unk, cmd_buf(4, 4))
                cert_tree:add_le(dload_cert_kid, cmd_buf(8, 4))
            elseif cmd_type == 0x10003 and cmd_len >= 0x8 then
                ctrl_tree = subtree:add(dload_ctrl, cmd_buf)
                ctrl_id = cmd_buf(0, 4):le_uint()
                ctrl_tree:add_le(dload_ctrl_id, cmd_buf(0, 4))
                ctrl_tree:add_le(dload_ctrl_len, cmd_buf(4, 4))
                ctrl_tree:add(dload_ctrl_data, cmd_buf(8))
                cmd_info = string.format(" (0x%x)", ctrl_id)
            else
                subtree:add(dload_unparsed, cmd_buf):set_text(bb6_tlv_table[cmd_type] or string.format("Unknown 0x%08x", cmd_type))
            end

            pinfo.cols.info:append((bb6_tlv_table[cmd_type] or string.format("Unknown 0x%08x")) .. cmd_info .. ", ")

            cur_pos = cur_pos + cmd_len + 0x8
        end
    elseif msg_type == 0xbb0bb001 then
        echo_len = length - 0x8
        pinfo.cols.info:append(string.format(" (%i bytes)", echo_len))
        subtree:add(dload_echo, buffer(8, echo_len))
    elseif msg_type == 0xbb0bb004 and msg_length == 0x10 then
        local vertree = subtree:add(dload_ver, buffer(8, 0x10))
        vertree:add(dload_ver_unk, buffer(0x8, 0xC))
        vertree:add_le(dload_ver_timestamp, buffer(0x14, 0x4))
    end
end

DissectorTable.get("usb.product"):add(0x042105ee, dload_protocol)
