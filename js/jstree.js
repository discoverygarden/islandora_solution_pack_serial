/**
 * @file
 * Create a jsTree for use in rendering a serial object.
 */

Drupal.settings.jsTree_jQuery = jQuery.noConflict(true);
(function ($) {
    Drupal.behaviors.jsTree = {
        attach: function(context, settings) {
            $('#islandora_serial_object_tree_view').jstree({
                'core': {
                    'data': settings.islandora_serial_object.data
                },
                'types': settings.islandora_serial_object.types,
                'plugins' : settings.islandora_serial_object.plugins,
                'contextmenu' : {
                    'items': function (node) {
                        if (node.type != 'stub') {
                            return {
                                'Goto': {
                                    'label': Drupal.t('Go to'),
                                    'action': function (obj) {
                                        if (node.type == 'pdf') {
                                           window.location = Drupal.settings.basePath + 'islandora/object/' + node.id + '/datastream/OBJ/view/', 'Islandora Object', 'window settings';
                                        }
                                        else {
                                           window.location = Drupal.settings.basePath + 'islandora/object/' + node.id, 'Islandora Object', 'window settings';
                                        }
                                    }
                                }
                            }
                        }
                        else {
                            return {}
                        }
                    }
                }
            });
            $('.islandora_serial_object_tree_close_all').click(function() {
                $('#islandora_serial_object_tree_view').jstree("close_all");
            });
        }
    };
})(Drupal.settings.jsTree_jQuery);
