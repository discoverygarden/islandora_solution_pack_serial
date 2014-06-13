/**
 * @file
 * Fire events on dragging yay.
 */
(function ($) {
    Drupal.behaviors.updateDragValues = {
        attach: function(context, settings) {
            if (typeof settings.islandoraSerialObjectModifiedIntermediates == "undefined") {
                settings.islandoraSerialObjectModifiedIntermediates = [];
            }
            else {
                $.each(settings.islandoraSerialObjectModifiedIntermediates, function(index, value) {
                    $(':input[name="' + value + '"]').addClass('islandora-serial-object-modified-intermediate');
                });
            }
            if (Drupal.tableDrag['islandora-serial-object-intermediate-table']) {
                Drupal.tableDrag['islandora-serial-object-intermediate-table'].onDrop = function() {
                    var dropped_row = this.rowObject;
                    var intermediate_levels = settings.islandoraSerialObjectIntermediateLevels;
                    var indent = this.rowObject.indents + 1;
                    if (indent in intermediate_levels) {
                        $(this.oldRowElement).find('input.islandora-serial-object-caption').not('.islandora-serial-object-modified-intermediate').val(intermediate_levels[indent].caption);
                    }
                    $(this.rowObject.children).each(function (index, element) {
                       var indent = $(".indentation", element).length + 1;
                        if (indent in intermediate_levels) {
                            $(element).find('input.islandora-serial-object-caption').not('.islandora-serial-object-modified-intermediate').val(intermediate_levels[indent].caption);
                        }
                    });
                }
            }
            $('input.islandora-serial-object-caption').keyup(function(object) {
               $(object.target).addClass('islandora-serial-object-modified-intermediate');
               if (settings.islandoraSerialObjectModifiedIntermediates.indexOf(object.target.name) == -1) {
                   settings.islandoraSerialObjectModifiedIntermediates.push(object.target.name);
               }
            });
        }
    }
})(jQuery);
