# EN: Require a complete set of previously generated C++ and shader
#     files. This function deliberately never generates mock guest inputs.
# PT-BR: Exige o conjunto completo de C++ e shaders ja gerados.
#     A funcao nunca cria dados ficticios do jogo.
function(unleashed_recomp_verify_pregenerated_guest_sources)
    set(_missing "")
    foreach(_path IN LISTS ARGN)
        if(NOT EXISTS "${_path}" OR IS_DIRECTORY "${_path}")
            list(APPEND _missing "${_path}")
        else()
            # EN: A zero-byte source is a broken generation artifact.
            # PT-BR: Um arquivo gerado vazio indica um artefato invalido.
            file(SIZE "${_path}" _size)
            if(_size EQUAL 0)
                list(APPEND _missing "${_path} (empty / vazio)")
            endif()
        endif()
    endforeach()

    if(_missing)
        list(LENGTH _missing _count)
        set(_preview "${_missing}")
        if(_count GREATER 12)
            list(SUBLIST _missing 0 12 _preview)
        endif()
        string(JOIN "\n  " _details ${_preview})
        message(FATAL_ERROR
            "Missing ${_count} pre-generated guest source(s) / "
            "Faltam ${_count} fonte(s) guest pre-gerados:\n  ${_details}\n"
            "Restore the locally generated PPC/shader files OR disable "
            "UNLEASHED_RECOMP_USE_PREGENERATED_GUEST_SOURCES and rebuild "
            "with your legally obtained default.xex/default.xexp/shader.ar.\n"
            "Restaure os fontes PPC/shaders criados anteriormente OU desligue "
            "a opcao e compile com os arquivos originais da sua copia.")
    endif()
endfunction()
