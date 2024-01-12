import IPython
from IPython.core.magic import Magics, cell_magic, magics_class, line_cell_magic, needs_local_scope
@magics_class
class compile_and_execute_cpp(Magics):
    @needs_local_scope
    @cell_magic
    def cpp(self, line, cell="", local_ns=None):
        """
        Compile, execute C++ code, and return the standard output.
        """
        # Write the cell contents to a file
        with open('tmp.cpp', 'w') as f:
            f.write(cell)

        # Execute the code
        import subprocess
        success = False
        try:
            output = subprocess.check_output(['g++', 'tmp.cpp', '-o', 'tmp.exe'], stderr=subprocess.STDOUT)
            output += subprocess.check_output(['./tmp.exe'], stderr=subprocess.STDOUT)
            print(output.decode('utf-8'))
            success = True
        except subprocess.CalledProcessError as e:
            print(e.output.decode('utf-8'))

        # Remove the temporary file
        import os
        os.remove('tmp.cpp')
        if success:
            os.remove('tmp.exe')

IPython.get_ipython().register_magics(compile_and_execute_cpp())