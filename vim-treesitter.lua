local cpath = package.cpath
package.cpath = cpath .. ";" .. vim.fn.expand("~/Developer/Projects/vim-treesitter/build/?.so")
local module = require("treesitter")

package.cpath = cpath

local props = {}
local _buffers = {}

local scope_hl_map = {
	{ "identifier", "Identifier" },
	{ "primitive", "StorageClass" },
	{ "string", "String" },
	{ "number", "Number" },
	{ "include", "Include" },
	{ "define", "Define" },
	{ "preproc", "PreProc" },
	{ "struct", "Structure" },
	{ "return", "Keyword" },
	{ "expression", "Operator" },
	{ "call", "Function" },
	{ "comment", "Comment" },
}

local function buffers(id)
	if not _buffers[id] then
		_buffers[id] = {
			number = id,
			last_start = 0,
		}
	end
	return _buffers[id]
end

function ts_parse_buffer()
	-- vim.command"syn off"

	local buf = vim.buffer()
	local n = buf.number

	local r = vim.window().line
	local c = vim.window().column

	local lc = #buf
	local sr = vim.window().height -- screen rows

	local ls = r - sr
	local le = r + sr

	if ls < 0 then
		ls = 0
	end
	if le > lc then
		le = lc
	end

	local b = buffers(n)
	b.last_start = ls

	-- todo .. query buffer range
	local s = ""
	for i = ls, le - 1, 1 do
		s = s .. buf[i + 1] .. "\n"
	end

	local fn = vim.fn.expand("%")
	local ext = fn:match("^.+(%..+)$")

	module.parse_buffer(s, string.len(s), ext, n)

	for i = ls, le - 1, 1 do
		local nodes = module.query_tree(n, i - ls, 0)
		vim.command("call prop_clear(" .. (i + 1) .. ")")
		for j, node in ipairs(nodes) do
			local start_row = math.floor(node[1])
			local start_column = math.floor(node[2])
			local end_row = math.floor(node[3])
			local end_column = math.floor(node[4])
			local node_type = node[5]

			if i + 1 == r and start_column + 1 < c and end_column > c then
				print(node_type)
			end

			local hl = nil
			for j, map in ipairs(scope_hl_map) do
				if string.find(node_type, map[1]) then
					hl = map[2]
				end
			end

			if hl and start_row == end_row then
				if not props[hl] then
					vim.command("call prop_type_add('" .. hl .. "', { 'highlight': '" .. hl .. "', 'priority': 999 })")
					props[hl] = true
				end
				vim.command(
					"call prop_add("
						.. i + 1
						.. ","
						.. start_column + 1
						.. ", { 'length': "
						.. end_column - start_column
						.. ", 'type': '"
						.. hl
						.. "'})"
				)
			end

			if start_row ~= end_row and i + 1 == r and start_column + 1 == c then
				print(
					ls + start_row + 1
						.. ","
						.. start_column + 1
						.. " - "
						.. ls + end_row + 1
						.. ","
						.. end_column
						.. " "
						.. node_type
				)
			end
		end
	end

	-- vim.command"q"
end

function ts_inner_block()
	-- local buf = vim.buffer()
	-- local n = buf.number

	-- local r = vim.window().line
	-- local c = vim.window().column
end

function ts_outer_block()
	local buf = vim.buffer()
	local n = buf.number

	local r = vim.window().line
	local c = vim.window().column

	local b = buffers(n)
	local ls = b.last_start
	local nodes = module.query_tree(n, r - ls, 1)

	for j, node in ipairs(nodes) do
		local start_row = math.floor(node[1])
		local start_column = math.floor(node[2])
		local end_row = math.floor(node[3])
		local end_column = math.floor(node[4])
		local node_type = node[5]
		print(
			ls + start_row + 1
				.. ","
				.. start_column + 1
				.. " - "
				.. ls + end_row + 1
				.. ","
				.. end_column
				.. " "
				.. node_type
		)
	end
end

function ts_log_tree()
	module.log_tree()
end

vim.command("au BufEnter * :lua ts_parse_buffer()")
vim.command("au CursorMoved,CursorMovedI * :lua ts_parse_buffer()")
vim.command("au TextChanged,TextChangedI * :lua ts_parse_buffer()")
vim.command("command TSInnerBlock 0 % :lua ts_inner_block()")
vim.command("command TSOuterBlock 0 % :lua ts_outer_block()")
vim.command("command TSLogTree 0 % :lua ts_log_tree()")
