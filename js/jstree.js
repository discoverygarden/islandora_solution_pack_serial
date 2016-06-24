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
                'plugins' : settings.islandora_serial_object.plugins
            });
            $('#islandora_serial_object_tree_view').bind("select_node.jstree", function(node, selected, event) {
                // For some reason jsTree doesn't pass the click through
                // in the event parameter, so grab it from selected instead.
                if (typeof selected.event != 'undefined') {
                    if (selected.event.type == 'click') {
                        window.location = Drupal.settings.basePath + 'islandora/object/' + selected.node.id;
                    }
                }
            });
            $('.islandora_serial_object_tree_close_all').click(function() {
                $('#islandora_serial_object_tree_view').jstree("close_all");
            });
            $('.islandora_serial_object_tree_expand_all').click(function() {
                $('#islandora_serial_object_tree_view').jstree("open_all");
            });
        }
    };
})(Drupal.settings.jsTree_jQuery);
