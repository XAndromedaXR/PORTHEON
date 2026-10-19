"""Generate Ivy Bridge compatibility plan for UFC 4 PS4 Port with instruction spanning."""
import argparse
import collections
import hashlib
import json
from pathlib import Path
import re
import struct
import sys

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / 'scripts'))

try:
    import capstone
    import keystone
except ImportError:
    print("ERROR: python packages 'capstone' and 'keystone-engine' are required to generate the plan.")
    print("Run: pip install capstone keystone-engine")
    sys.exit(1)

from link_modules import module
from cpu_compat import atomic_output, SUPPORTED

ASSEMBLER = keystone.Ks(keystone.KS_ARCH_X86, keystone.KS_MODE_64)

def canonical(reg):
    names = {'eax': 'rax', 'ebx': 'rbx', 'ecx': 'rcx', 'edx': 'rdx', 'esi': 'rsi',
             'edi': 'rdi', 'ebp': 'rbp', 'esp': 'rsp', 'ax': 'rax', 'bx': 'rbx',
             'cx': 'rcx', 'dx': 'rdx', 'si': 'rsi', 'di': 'rdi', 'bp': 'rbp', 'sp': 'rsp'}
    return names.get(reg, reg[:-1] if reg.startswith('r') and reg.endswith(('d', 'w')) else reg)

def word(reg, width):
    if width == 64: return reg
    if width == 16:
        return {'rax': 'ax', 'rbx': 'bx', 'rcx': 'cx', 'rdx': 'dx', 'rsi': 'si',
                'rdi': 'di', 'rbp': 'bp', 'rsp': 'sp'}.get(reg, reg + 'w')
    return {'rax': 'eax', 'rbx': 'ebx', 'rcx': 'ecx', 'rdx': 'edx', 'rsi': 'esi',
            'rdi': 'edi', 'rbp': 'ebp', 'rsp': 'esp'}.get(reg, reg + 'd')

def movbe_body(operands):
    args = operands.split(', ')
    memory_idx = 0 if '[' in args[0] else 1
    memory = re.fullmatch(r'(word|dword|qword) ptr \[([a-z0-9x +*\-]+)\]', args[memory_idx])
    width = {'word': 16, 'dword': 32, 'qword': 64}[memory[1]]
    reg = canonical(args[1 - memory_idx])
    addr_regs = re.findall(r'\b(?:r(?:1[0-5]|[8-9])|r(?:ax|bx|cx|dx|si|di|bp|sp|ip))\b', memory[2])
    scratch = next(r for r in ('r10', 'r11', 'r8', 'r9', 'rax', 'rdx', 'rsi')
                   if r != reg and r not in addr_regs)
    temp = word(scratch, width)
    depth = 128 + 8 + (8 if width == 16 else 0)
    adjusted = list(args)
    adjusted[memory_idx] = re.sub(r'\brsp\b', f'rsp + {depth}', args[memory_idx])
    lines = ['lea rsp, [rsp-128]', f'push {scratch}']
    if width == 16: lines.append('pushfq')
    lines += [f'mov {temp}, {adjusted[1]}',
              f'ror {temp}, 8' if width == 16 else f'bswap {temp}',
              f'mov {adjusted[0]}, {temp}']
    if width == 16: lines.append('popfq')
    lines += [f'pop {scratch}', 'lea rsp, [rsp+128]']
    return bytes(ASSEMBLER.asm('; '.join(lines))[0])

def body(mnemonic, operands):
    if mnemonic == 'movbe':
        return movbe_body(operands)
    args = operands.split(', ')
    dest = canonical(args[0])
    width = next(w for w in (16, 32, 64) if args[0] == word(dest, w))
    orig = [canonical(arg) for arg in args if '[' not in arg]
    temp = [r for r in ('r10', 'r11', 'r8', 'r9', 'rax', 'rdx', 'rsi', 'rdi', 'rbx', 'r12')
            if r not in orig and r != 'rcx'][:3]
    first, control, mask = [word(r, width) for r in temp]
    saved = temp + (['rcx'] if mnemonic == 'bextr' and dest != 'rcx' else [])
    adjusted = [re.sub(r'\brsp\b', f'rsp + {128 + 8 * len(saved)}', arg) if '[' in arg else arg for arg in args]
    lines = ['lea rsp, [rsp-128]'] + [f'push {r}' for r in saved]
    lines += [f'mov {first}, {adjusted[1]}']

    if mnemonic == 'andn':
        lines += [f'not {first}', f'and {first}, {adjusted[2]}']
    elif mnemonic in ('blsr', 'blsi', 'blsmsk'):
        lines += [f'test {first}, {first}', 'jz zero']
        if mnemonic == 'blsr':
            lines += [f'lea {mask}, [{temp[0]}-1]', f'and {first}, {mask}']
        elif mnemonic == 'blsi':
            lines += [f'mov {mask}, {first}', f'neg {first}', f'and {first}, {mask}', 'stc']
        else:
            lines += [f'lea {mask}, [{temp[0]}-1]', f'xor {first}, {mask}']
        lines += ['jmp done', 'zero:', f'xor {first}, {first}']
    elif mnemonic == 'bextr':
        lines += [f'mov {word(temp[1], 32)}, {word(canonical(args[2]), 32)}',
                  f'mov ecx, {word(temp[1], 32)}', 'and ecx, 255', f'cmp ecx, {width}',
                  'jae zero', f'shr {first}, cl', f'mov ecx, {word(temp[1], 32)}',
                  'shr ecx, 8', 'and ecx, 255', f'cmp ecx, {width}', 'jae nomask',
                  f'mov {mask}, -1', f'shl {mask}, cl', f'not {mask}',
                  f'and {first}, {mask}', 'jmp done', 'nomask:',
                  f'test {first}, {first}', 'jmp done', 'zero:', f'xor {first}, {first}']
    elif mnemonic in ('tzcnt', 'lzcnt'):
        lines += [f"{'bsf ' if mnemonic == 'tzcnt' else 'bsr '} {mask}, {first}", 'jz zero']
        if mnemonic == 'tzcnt':
            lines += [f'mov {first}, {mask}']
        else:
            lines += [f'mov {first}, {width - 1}', f'sub {first}, {mask}']
        lines += [f'test {first}, {first}', 'jmp done', 'zero:',
                  f'mov {first}, {width}', f'test {first}, {first}', 'stc']
    lines += ['done:', f'mov {args[0]}, {first}']
    lines += [f'pop {r}' for r in reversed(saved)] + ['lea rsp, [rsp+128]']
    assembled, _ = ASSEMBLER.asm('; '.join(lines))
    return bytes(assembled)

def function_ranges(mod):
    functions = set()
    header = next((p for p in mod['ph'] if p['type'] == 0x6474e550), None)
    if header:
        offset, addr = header['offset'], header['vaddr']
        count = struct.unpack_from('<I', mod['elf'], offset + 8)[0]
        for idx in range(count):
            start, fde = struct.unpack_from('<ii', mod['elf'], offset + 12 + idx * 8)
            fde_offset = next((p['offset'] + (addr + fde) - p['vaddr'] for p in mod['ph'] if p['type'] == 1 and p['vaddr'] <= addr + fde < p['vaddr'] + p['filesz']), None)
            if fde_offset:
                size = struct.unpack_from('<I', mod['elf'], fde_offset + 12)[0]
                if 0 < size < 4 * 1024**2:
                    functions.add((addr + start, size))
    for s in mod['symbols']:
        if s['type'] == 2 and s['section'] and s['size']:
            functions.add((s['value'], s['size']))
    return sorted(functions)

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('game', type=Path, help='Folder game UFC (CUSA14204)')
    p.add_argument('--out', type=Path, default=ROOT / 'tools/legacy-cpu-plan.json')
    args = p.parse_args()
    
    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
    detailed = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
    detailed.detail = True

    plan = {'version': 2, 'modules': {}, 'sites': [], 'unhandled': []}
    seen = set()

    for name, rel_path in [('eboot.bin', 'eboot.bin'), ('libc.prx', 'sce_module/libc.prx'),
                           ('libSceFios2.prx', 'sce_module/libSceFios2.prx')]:
        path = args.game / rel_path
        if not path.exists(): continue
        mod = module(path)
        plan['modules'][name] = mod['sha256']
        fn_list = function_ranges(mod)

        # Collect all incoming branch targets in this module
        branch_targets = set()
        for start, size in fn_list:
            seg = next((p for p in mod['ph'] if p['type'] == 1 and (p['flags'] & 1) and p['vaddr'] <= start < p['vaddr'] + p['filesz']), None)
            if not seg: continue
            off = seg['offset'] + start - seg['vaddr']
            for insn in detailed.disasm(bytes(mod['elf'][off:off+size]), start):
                if insn.group(capstone.CS_GRP_JUMP) or insn.group(capstone.CS_GRP_CALL):
                    for op_item in insn.operands:
                        if op_item.type == capstone.x86.X86_OP_IMM:
                            branch_targets.add(op_item.imm)

        for start, size in fn_list:
            seg = next((p for p in mod['ph'] if p['type'] == 1 and (p['flags'] & 1) and p['vaddr'] <= start < p['vaddr'] + p['filesz']), None)
            if not seg: continue
            off = seg['offset'] + start - seg['vaddr']
            fn_insns = list(detailed.disasm(bytes(mod['elf'][off:off+size]), start))
            
            for idx, insn in enumerate(fn_insns):
                addr = insn.address
                length = insn.size
                mnem = insn.mnemonic
                op = insn.op_str
                if mnem not in SUPPORTED or (name, addr) in seen: continue
                seen.add((name, addr))
                
                at = seg['offset'] + addr - seg['vaddr']
                if length >= 5:
                    try:
                        rep = body(mnem, op)
                        plan['sites'].append({
                            'module': name,
                            'address': addr,
                            'bytes': bytes(mod['elf'][at:at+length]).hex(),
                            'body': rep.hex(),
                            'instruction': f'{mnem} {op}',
                            'rip_fixups': []
                        })
                    except Exception as e:
                        plan['unhandled'].append({'module': name, 'address': hex(addr), 'instruction': f'{mnem} {op}', 'reason': str(e)})
                else:
                    # Short instruction (< 5 bytes)
                    # Special site: 0x186dc66 in eboot.bin (trampoline stub via alignment NOPs at 0x186dcc1)
                    if addr == 0x186dc66 and name == 'eboot.bin':
                        try:
                            rep = body(mnem, op)
                            plan['sites'].append({
                                'module': name,
                                'address': addr,
                                'bytes': bytes(mod['elf'][at:at+4]).hex(),
                                'patch': 'eb599090',
                                'stub_address': 0x186dcc1,
                                'body': rep.hex(),
                                'instruction': f'{mnem} {op} [stub via 0x186dcc1]',
                                'return_target': 0x186dc6a,
                                'rip_fixups': []
                            })
                            continue
                        except Exception as e:
                            pass

                    next_insn = fn_insns[idx + 1] if idx + 1 < len(fn_insns) else None
                    prev_insn = fn_insns[idx - 1] if idx > 0 else None
                    spanned = False

                    # Strategy 1: Forward span if next instruction is an unconditional jmp imm
                    if next_insn:
                        i1, i2 = insn, next_insn
                        span_len = i1.size + i2.size
                        is_rip = any(operand.type == capstone.x86.X86_OP_MEM and operand.mem.base == capstone.x86.X86_REG_RIP for operand in i2.operands)
                        if span_len >= 5 and not is_rip and i2.mnemonic == 'jmp' and len(i2.operands) == 1 and i2.operands[0].type == capstone.x86.X86_OP_IMM and i2.address not in branch_targets:
                            try:
                                rep1 = body(i1.mnemonic, i1.op_str)
                                combined_bytes = bytes(i1.bytes) + bytes(i2.bytes)
                                plan['sites'].append({
                                    'module': name,
                                    'address': addr,
                                    'bytes': combined_bytes.hex(),
                                    'body': rep1.hex(),
                                    'instruction': f'{i1.mnemonic} {i1.op_str} [spanned with jmp to {hex(i2.operands[0].imm)}]',
                                    'return_target': i2.operands[0].imm,
                                    'rip_fixups': []
                                })
                                spanned = True
                            except Exception:
                                pass

                        # Strategy 2: Forward span with safe non-branch instruction (i2 not a branch target)
                        elif span_len >= 5 and not is_rip and not (i2.group(capstone.CS_GRP_JUMP) or i2.group(capstone.CS_GRP_CALL)) and i2.address not in branch_targets:
                            try:
                                rep1 = body(i1.mnemonic, i1.op_str)
                                combined_body = rep1 + bytes(i2.bytes)
                                combined_bytes = bytes(i1.bytes) + bytes(i2.bytes)
                                plan['sites'].append({
                                    'module': name,
                                    'address': addr,
                                    'bytes': combined_bytes.hex(),
                                    'body': combined_body.hex(),
                                    'instruction': f'{i1.mnemonic} {i1.op_str} [spanned with {i2.mnemonic}]',
                                    'rip_fixups': []
                                })
                                spanned = True
                            except Exception:
                                pass

                    # Strategy 3: Backward span with prev instruction if forward span wasn't safe
                    if not spanned and prev_insn:
                        i0, i1 = prev_insn, insn
                        span_len = i0.size + i1.size
                        i0_rip = any(operand.type == capstone.x86.X86_OP_MEM and operand.mem.base == capstone.x86.X86_REG_RIP for operand in i0.operands)
                        i0_branch = i0.group(capstone.CS_GRP_JUMP) or i0.group(capstone.CS_GRP_CALL)
                        if span_len >= 5 and not i0_rip and not i0_branch and i0.address not in branch_targets and i1.address not in branch_targets:
                            try:
                                rep1 = body(i1.mnemonic, i1.op_str)
                                combined_body = bytes(i0.bytes) + rep1
                                combined_bytes = bytes(i0.bytes) + bytes(i1.bytes)
                                plan['sites'].append({
                                    'module': name,
                                    'address': i0.address,
                                    'bytes': combined_bytes.hex(),
                                    'body': combined_body.hex(),
                                    'instruction': f'{i1.mnemonic} {i1.op_str} [backward spanned with {i0.mnemonic}]',
                                    'return_target': i1.address + i1.size,
                                    'rip_fixups': []
                                })
                                spanned = True
                            except Exception:
                                pass

                    if not spanned:
                        plan['unhandled'].append({'module': name, 'address': hex(addr), 'instruction': f'{mnem} {op}', 'reason': 'Short instruction could not be spanned'})

    args.out.parent.mkdir(parents=True, exist_ok=True)
    with atomic_output(args.out) as s:
        s.write(json.dumps(plan, separators=(',', ':')).encode('utf-8'))
    print(f"Legacy CPU Plan created: {len(plan['sites'])} sites translated, {len(plan['unhandled'])} unhandled.")

if __name__ == '__main__':
    main()
