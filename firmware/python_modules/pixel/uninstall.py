import uos as os, time, machine, system
import uinterface, woezel, rgb

app = machine.nvs_getstr('launcher', 'uninstall_name')
app_file = machine.nvs_getstr('launcher', 'uninstall_file')

if (not app) or not (app_file):
    system.home()

agreed = uinterface.confirmation_dialog('Uninstall \'%s\'?' % app)
if not agreed:
    system.home()

uinterface.loading_text("Removing " + app + "...")
install_path = woezel.get_install_path()
print(app)
print(app_file)
print(install_path)
for rm_file in os.listdir("%s/%s" % (install_path, app_file)):
    os.remove("%s/%s/%s" % (install_path, app_file, rm_file))
os.rmdir("%s/%s" % (install_path, app_file))

machine.nvs_setstr('launcher', 'uninstall_name', '')
machine.nvs_setstr('launcher', 'uninstall_file', '')

rgb.clear()
uinterface.skippabletext("Uninstall completed!")
system.home()
